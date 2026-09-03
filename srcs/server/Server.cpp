/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:06:07 by ratanaka          #+#    #+#             */
/*   Updated: 2026/09/03 14:03:08 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Router.hpp"
#include "../../includes/Exeptions.hpp"
#include "../../includes/DeleteHandler.hpp"
#include "../../includes/StaticHandler.hpp"
#include "../../includes/ErrorBuilder.hpp"
#include "../../includes/HttpParser.hpp"

#include <arpa/inet.h>		// inet_addr
#include <fcntl.h>			// fcntl, F_SETFL, O_NONBLOCK
#include <netinet/in.h>		// sockaddr_in, htons
#include <sys/epoll.h>		// epoll_create, epoll_ctl, epoll_wait, EPOLLIN/OUT
#include <sys/socket.h>		// bind, listen, accept, socklen_t
#include <unistd.h>			// close

#include <cctype>			// tolower
#include <cerrno>			// errno
#include <cstring>			// strerror, memset
#include <ctime>			// time, time_t
#include <iostream>			// cout, endl
#include <signal.h>
#include <sstream>
#include <sys/wait.h>
#include "../../includes/CgiHandler.hpp"

namespace {
	// sig_atomic_t: só isso é seguro de tocar dentro de um signal handler.
	volatile sig_atomic_t g_shutdown = 0;
	void handleShutdownSignal(int) { g_shutdown = 1; }
}

//================================
//			Constructors		//
//================================

Server::Server(Config config) : _epollFd(-1), _config(config) {}

Server::~Server() {
	for (size_t i = 0; i < _sockets.size(); i++){
		close(_sockets[i]->getFd());
		delete _sockets[i];
	}
	if (_epollFd != -1)
		close(_epollFd);
}

bool Server::isServerFd(int fd) {
	for (size_t i = 0; i < _serverFds.size(); i++) {
		if (_serverFds[i] == fd)
			return true;
	}
	return false;
}

//================================
//			Functions			//
//================================

void	Server::initServer() {
	signal(SIGINT, handleShutdownSignal);
	signal(SIGTERM, handleShutdownSignal);
	signal(SIGPIPE, SIG_IGN);

    _epollFd = epoll_create(10);
	if (_epollFd == -1)
		throw ServerException(std::string("epoll_create failed -> ") + strerror(errno));
	if (fcntl(_epollFd, F_SETFD, FD_CLOEXEC) == -1)
		throw ServerException(std::string("epoll FD_CLOEXEC failed -> ") + strerror(errno));

	std::vector<ServerConfig> _servers = _config.getServers();
	for (size_t i = 0; i < _servers.size(); i++){
		int		currentPort = _servers[i].getPort();
		Socket*	newSocket = new Socket();
		int		fd = newSocket->getFd();
	
		struct	sockaddr_in address;
		std::memset(&address, 0, sizeof(address));
		address.sin_family		= AF_INET; // usando IPv4
		address.sin_port		= htons(currentPort); // htons é um tradutor (portavai ser 8080)
		std::string host = _servers[i].getHost();
		if (host.empty())
			host = "0.0.0.0";
		address.sin_addr.s_addr	= inet_addr(host.c_str());
	
		//bind() -> este socket vai receber conexoes na porta 8080
		if (bind(fd, (struct sockaddr*)&address, sizeof(address)) == -1){
			// newSocket ainda não foi pra _sockets (só entra na linha de baixo,
			// depois do bind/listen passarem), então o destructor do Server não
			// ia limpar esse fd/objeto no unwind da exceção -- fechar e deletar
			// aqui à mão antes de lançar. Sem isso, ASan/LeakSanitizer acusa
			// "Direct leak" nesse Socket toda vez que um bind() falha (ex.: duas
			// server{} na mesma interface:porta, ou porta já em uso).
			close(fd);
			delete newSocket;
			throw ServerException(std::string("bind() system call failed -> ") + strerror(errno)); }
		if (listen(fd, 10) == -1){
			close(fd);
			delete newSocket;
			throw ServerException(std::string("listen() system call failed -> ") + strerror(errno));}
	
		_sockets.push_back(newSocket);
		_serverFds.push_back(fd);
	
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = fd;
		if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
			throw ServerException(std::string("epoll_ctl failed -> ") + strerror(errno));

		std::cout << "Server listening on port " << currentPort << " (fd: " << fd << ")" << std::endl;
	}
}

void Server::handleNewConnection(int serverFd){
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientFd == -1) return;

    fcntl(clientFd, F_SETFL, O_NONBLOCK);
    fcntl(clientFd, F_SETFD, FD_CLOEXEC);

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = clientFd;
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientFd, &event) == -1){
        close(clientFd);
        return;
    }
    
    _clients[clientFd] = new Client(clientFd);
    _clients[clientFd]->setServerFd(serverFd);
    // seta o client_max_body_size do server{} já aqui, no accept -- é o
    // primeiro momento em que dá pra saber (o socket de listen decide qual
    // server{} é esse) e deixa Client::readData() cortar o body cedo, em vez
    // de só descobrir o limite depois que tudo já foi lido.
    _clients[clientFd]->setMaxBodySize(findServerConfig(serverFd).getMaxBodySize());
    std::cout << "New client connected on fd: " << clientFd << std::endl;
}

/*
	Acha o ServerConfig (o "server{}" do config file) que corresponde ao
	socket de listen que aceitou esse client. _serverFds e _config.getServers()
	são preenchidos juntos, na mesma ordem, dentro de initServer().
*/
const ServerConfig& Server::findServerConfig(int serverFd) const
{
	const std::vector<ServerConfig>& servers = _config.getServers();

	for (size_t i = 0; i < _serverFds.size(); i++)
	{
		if (_serverFds[i] == serverFd)
			return servers[i];
	}
	return servers[0]; // fallback: não devia acontecer, mas evita crash
}

// waitpid(pid, NULL, 0) bloqueia até o processo terminar de verdade -- num
// server single-threaded rodando dentro de um handler de epoll, isso trava
// TODO o loop (todo cliente, toda conexão) até aquele filho específico ser
// colhido pelo kernel. Mesmo depois de SIGKILL isso não é instantâneo
// garantido (processo em D state, por exemplo). WNOHANG nunca bloqueia: se o
// processo já morreu, colhe na hora; se não, guarda o pid pra tentar de novo
// no próximo reapPendingChildren() em vez de travar esperando.
void Server::reapChild(pid_t pid) {
	if (waitpid(pid, NULL, WNOHANG) == 0)
		_pendingReap.push_back(pid);
}

void Server::reapPendingChildren() {
	std::vector<pid_t> stillPending;

	for (size_t i = 0; i < _pendingReap.size(); i++) {
		pid_t pid = _pendingReap[i];
		pid_t result = waitpid(pid, NULL, WNOHANG);
		if (result == 0)
			stillPending.push_back(pid); // ainda rodando, tenta na próxima volta
		// result == pid (colhido) ou -1 (ex.: ECHILD, já não existe mais) -> descarta
	}
	_pendingReap = stillPending;
}

void Server::removeClient(int fd) {
	if (_clients.find(fd) == _clients.end()) return;

	Client* client = _clients[fd];
	if (client->getCgiContext().pid != -1
		|| client->getCgiContext().stdout_fd != -1
		|| client->getCgiContext().stdin_fd != -1){

		int pipeOutFd = client->getCgiContext().stdout_fd;
		if (pipeOutFd != -1) {
			epoll_ctl(_epollFd, EPOLL_CTL_DEL, pipeOutFd, NULL);
			close(pipeOutFd);
			_cgiPipes.erase(pipeOutFd);
		}

		int pipeInFd = client->getCgiContext().stdin_fd;
		if (pipeInFd != -1) {
			epoll_ctl(_epollFd, EPOLL_CTL_DEL, pipeInFd, NULL);
			close(pipeInFd);
			_cgiWritePipes.erase(pipeInFd);
		}

		if (client->getCgiContext().pid != -1) {
			kill(client->getCgiContext().pid, SIGKILL);
			reapChild(client->getCgiContext().pid);
			client->getCgiContext().pid = -1;
		}
	}

    epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    delete client;
    _clients.erase(fd);
}


static std::string resolveFilePath(const LocationConfig& loc, const ServerConfig& server, const std::string& uri)
{
	std::string root = loc.getRoot().empty() ? server.getRoot() : loc.getRoot();
	if (root.empty())
		root = "./www";
	if (!root.empty() && root[root.size() - 1] == '/')
		root.erase(root.size() - 1);
	return root + uri;
}

static std::string resolveCustomErrorPagePath(const LocationConfig& loc, const ServerConfig& server,
	const std::string& pagePath)
{
	if (pagePath.empty())
		return "";
	if (pagePath[0] == '/' || pagePath.compare(0, 2, "./") == 0)
		return pagePath;

	std::string root = loc.getRoot().empty() ? server.getRoot() : loc.getRoot();
	if (root.empty())
		root = "./www";
	if (!root.empty() && root[root.size() - 1] == '/')
		root.erase(root.size() - 1);
	return root + "/" + pagePath;
}

static std::string buildRedirectResponse(int statusCode, const std::string& target)
{
	std::ostringstream oss;
	std::string message = (statusCode == 301) ? "Moved Permanently" : "Found";
	std::string body = "<html><body><h1>" + message + "</h1></body></html>";
	oss << "HTTP/1.1 " << statusCode << " " << message << "\r\n"
		<< "Location: " << target << "\r\n"
		<< "Content-Type: text/html; charset=UTF-8\r\n"
		<< "Content-Length: " << body.length() << "\r\n"
		<< "\r\n"
		<< body;
	return oss.str();
}

void Server::dispatchRequest(int currentFd, Client* client)
{
	// achado antes do parse-error também, porque agora o 400 já precisa
	// dele pra tentar resolver uma error_page 400 custom (só existe
	// server{} nesse ponto, a request ainda nem foi roteada pra location
	// nenhuma).
	const ServerConfig& serverConfig = findServerConfig(client->getServerFd());

	// Client::readData() já cortou a leitura assim que percebeu que o body
	// ia estourar client_max_body_size -- _rawRequest pode estar com o body
	// incompleto de propósito, então nem vale tentar dar parse nele: já sabe
	// que a resposta é 413.
	if (client->isBodyTooLarge())
	{
		std::cout << "[DISPATCH] Body too large, cut off during read (before buffering it all)" << std::endl;
		std::string customErrorPage413 = "";
		const std::map<int, std::string>& serverErrorPages413 = serverConfig.getErrorPages();
		if (serverErrorPages413.find(413) != serverErrorPages413.end())
			customErrorPage413 = ErrorBuilder::resolvePagePath(serverConfig.getRoot(), serverErrorPages413.find(413)->second);
		client->setResponse(ErrorBuilder::build(413, customErrorPage413));
		client->setState(Client::WRITING);

		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = currentFd;
		epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentFd, &event);
		return;
	}

	HttpParser parser;
	HttpParser::State parseState = parser.parse(client->getRawRequest(), client->getRequest());

	if (parseState == HttpParser::ERROR)
	{
		// request line ou headers malformados -> 400, nem tenta rotear
		std::string customErrorPage400 = "";
		const std::map<int, std::string>& serverErrorPages = serverConfig.getErrorPages();
		if (serverErrorPages.find(400) != serverErrorPages.end())
			customErrorPage400 = ErrorBuilder::resolvePagePath(serverConfig.getRoot(), serverErrorPages.find(400)->second);
		client->setResponse(ErrorBuilder::build(400, customErrorPage400));
		client->setState(Client::WRITING);

		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = currentFd;
		epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentFd, &event);
		return;
	}

	const std::string& method = client->getRequest().getMethod();
	const std::string& uri = client->getRequest().getUri();

	// HTTP/1.1 é keep-alive por padrão a menos que o client peça "close";
	// HTTP/1.0 (ou qualquer coisa fora 1.1) é o oposto -- fecha por padrão a
	// menos que o client peça "keep-alive" explícito. Setado aqui, antes de
	// qualquer handler montar resposta: StaticHandler manda "Connection:
	// keep-alive" incondicionalmente sem olhar a request, então sem isso um
	// GET HTTP/1.0 comum (curl/ab/siege sem -k/-b) ficava preso numa conexão
	// que ele nunca pediu pra manter aberta -- Client::setResponse usa esse
	// valor como decisão final quando a resposta não diz "close" ela mesma.
	std::string connectionHeader = client->getRequest().getHeader("Connection");
	for (size_t i = 0; i < connectionHeader.size(); ++i)
		connectionHeader[i] = static_cast<char>(std::tolower(connectionHeader[i]));
	bool clientWantsClose = connectionHeader.find("close") != std::string::npos;
	bool clientWantsKeepAlive = connectionHeader.find("keep-alive") != std::string::npos;
	bool isHttp11 = (client->getRequest().getVersion() == "HTTP/1.1");
	client->setKeepAliveEligible(!clientWantsClose && (isHttp11 || clientWantsKeepAlive));

	// matchLoc subiu pra antes do check de 413 (era só declarado lá embaixo)
	// pra dar pro 413 acesso à error_page da location, igual 403/404/405/500
	// já fazem -- não tem custo, matchLoc não tem efeito colateral nenhum.
	LocationConfig loc = Router::matchLoc(serverConfig, uri);

	// 413 antes de qualquer roteamento: se o body já veio maior que o
	// limite do server{}, nem vale a pena decidir CGI/upload/etc.
	const std::string& body = client->getRequest().getBody();
	if (!body.empty() && body.size() > serverConfig.getMaxBodySize())
	{
		std::cout << "[DISPATCH] Body too large: " << body.size()
			<< " > " << serverConfig.getMaxBodySize() << std::endl;
		std::string customErrorPage413 = "";
		const std::map<int, std::string>& errorPages413 = loc.getErrorPages();
		if (errorPages413.find(413) != errorPages413.end())
			customErrorPage413 = resolveCustomErrorPagePath(loc, serverConfig, errorPages413.find(413)->second);
		client->setResponse(ErrorBuilder::build(413, customErrorPage413));
		client->setState(Client::WRITING);

		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = currentFd;
		epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentFd, &event);
		return;
	}

	if (loc.getReturnCode() != 0 && !loc.getReturnPath().empty())
	{
		client->setResponse(buildRedirectResponse(loc.getReturnCode(), loc.getReturnPath()));
		client->setState(Client::WRITING);
		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = currentFd;
		epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentFd, &event);
		return;
	}

	std::string filePath = resolveFilePath(loc, serverConfig, uri);
	RouteType type = Router::classify(loc, filePath, method);

	bool readyToWrite = false;

	if (type == ERROR)
	{
		// método não permitido na location -> 405
		std::string customErrorPage = "";
		const std::map<int, std::string>& errorPages = loc.getErrorPages();
		if (errorPages.find(405) != errorPages.end())
			customErrorPage = resolveCustomErrorPagePath(loc, serverConfig, errorPages.find(405)->second);
		client->setResponse(ErrorBuilder::build(405, customErrorPage));
		client->setState(Client::WRITING);
		readyToWrite = true;
	}
	else if (type == CGI)
	{
		CgiHandler cgi;
		cgi.handle(client->getRequest(), loc, *client);
		if (client->getState() == Client::CGI_RUNNING)
		{
			int pipeOut = client->getCgiContext().stdout_fd;
			if (pipeOut != -1)
			{
				struct epoll_event ev;
				ev.events = EPOLLIN; // Queremos LER o que o CGI escreve
				ev.data.fd = pipeOut;
				epoll_ctl(_epollFd, EPOLL_CTL_ADD, pipeOut, &ev);
				_cgiPipes[pipeOut] = client;
			}

			int pipeIn = client->getCgiContext().stdin_fd;
			if (pipeIn != -1)
			{
				struct epoll_event ev;
				ev.events = EPOLLOUT; // Queremos ESCREVER pro CGI (POST)
				ev.data.fd = pipeIn;
				epoll_ctl(_epollFd, EPOLL_CTL_ADD, pipeIn, &ev);
				_cgiWritePipes[pipeIn] = client;
			}
		}
	}
	else if (method == "DELETE")
	{
		DeleteHandler del;
		readyToWrite = del.handle(client->getRequest(), loc, *client);
	}
	else if (method == "POST")
	{
		UploadHandler upload;
		readyToWrite = upload.handle(client->getRequest(), loc, *client);
	}
	else if (method == "HEAD")
	{
		StaticHandler st;
		readyToWrite = st.handle(client->getRequest(), loc, *client);
	}
	else // STATIC ou DIR -> StaticHandler decide (index/autoindex/404/etc)
	{
		StaticHandler st;
		readyToWrite = st.handle(client->getRequest(), loc, *client);
	}

	if (readyToWrite)
	{
		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = currentFd;
		epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentFd, &event);
	}
}

void Server::serverLoop(){
    const int MAX_EVENTS = 64;
    struct epoll_event events[MAX_EVENTS];

    while (!g_shutdown){
        int numEvents = epoll_wait(_epollFd, events, MAX_EVENTS, 2000);
        if(numEvents == -1) continue; // EINTR (ex.: SIGINT) cai aqui e o while checa g_shutdown de novo

        for (int i = 0; i < numEvents; i++){
            int currentFd = events[i].data.fd;

			if (_cgiPipes.find(currentFd) != _cgiPipes.end()) {
                handleCgiOutput(currentFd); // O Python falou connosco!
				continue;
            }
			if (_cgiWritePipes.find(currentFd) != _cgiWritePipes.end()) {
				if (events[i].events & EPOLLOUT) {handleCgiWrite(currentFd);}
				continue;
			}
			if (isServerFd(currentFd) && (events[i].events & EPOLLIN)) {
				handleNewConnection(currentFd);
				continue;
			}
			else if (events[i].events & EPOLLIN) {
				Client* client = _clients[currentFd];

				if (client->readData() == true) {
					if (client->getState() == Client::CLOSED) {
						removeClient(currentFd);
					}
					else {
						dispatchRequest(currentFd, client);
					}
				}
			}
					else if (events[i].events & EPOLLOUT) {
				if (_cgiWritePipes.count(currentFd)) {
                    handleCgiWrite(currentFd);
                } else {
					// Manda o Cliente enviar os próprios dados
					Client* client = _clients[currentFd];
					if (client->writeData() == true) {
						if (client->getState() == Client::CLOSED || client->shouldClose()) {
							removeClient(currentFd);
						} else {
							client->prepareNextRequest();
							struct epoll_event event;
							event.events = EPOLLIN;
							event.data.fd = currentFd;
							epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentFd, &event);
						}
					}
				}
            }
        }
        checkTimeouts();
        reapPendingChildren();
    }

	std::cout << "\n[SERVER] Shutdown signal received, closing connections..." << std::endl;
	std::vector<int> pendingFds;
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		pendingFds.push_back(it->first);
	for (size_t i = 0; i < pendingFds.size(); i++)
		removeClient(pendingFds[i]);
}

void Server::checkTimeouts() {
    time_t now = time(NULL);
    std::map<int, Client*>::iterator it = _clients.begin();
    
    while (it != _clients.end()) {
        Client* client = it->second;
		int clientFd = it->first;
        
		if (client->getState() == Client::CGI_RUNNING) {
			if (now - client->getCgiContext().startTime > 30) {
				std::cout << "[TIMEOUT 504] CGI script on fd " << clientFd << " timed out. Killing process!" << std::endl;
				if (client->getCgiContext().pid != -1){
					kill(client->getCgiContext().pid, SIGKILL);
					// sem isso o pid virava zumbio até o cliente eventualmente
					// desconectar e passar por removeClient() -- que também
					// reapa, mas não devia ser o único lugar que faz isso.
					reapChild(client->getCgiContext().pid);
					client->getCgiContext().pid = -1;
				}

				int pipeFd = client->getCgiContext().stdout_fd;
				if (pipeFd != -1) {
					epoll_ctl(_epollFd, EPOLL_CTL_DEL, pipeFd, NULL);
					close(pipeFd);
					_cgiPipes.erase(pipeFd);
					client->getCgiContext().stdout_fd = -1;
				}
				
				int pipeInFd = client->getCgiContext().stdin_fd;
				if (pipeInFd != -1) {
					epoll_ctl(_epollFd, EPOLL_CTL_DEL, pipeInFd, NULL);
					close(pipeInFd);
					_cgiWritePipes.erase(pipeInFd);
					client->getCgiContext().stdin_fd = -1;
				}
				
				std::string error504 = "HTTP/1.1 504 Gateway Timeout\r\nConnection: close\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: 47\r\n\r\n<html><body><h1>504 Gateway Timeout</h1></body></html>";
				client->setResponse(error504);
				client->setState(Client::WRITING);

				struct epoll_event event;
				event.events = EPOLLOUT;
				event.data.fd = clientFd;
				epoll_ctl(_epollFd, EPOLL_CTL_MOD, clientFd, &event);
			}
			++it;
		}
        else if (client->isTimeout(now, 60)) { // 60 seconds
            std::cout << "[TIMEOUT] Client on fd " << it->first << " disconnected!" << std::endl;
            
            int fdToErase = it->first;
            ++it;
            removeClient(fdToErase);
        } else {
            ++it;
        }
    }
}

void Server::handleCgiWrite(int pipeFd) {
	Client* client = _cgiWritePipes[pipeFd];
	const std::string& body = client->getRequest().getBody();
	size_t sent = client->getCgiContext().inputSent;
	
	if (sent < body.length()) {
		int bytesWritten = write(pipeFd, body.c_str() + sent, body.length() - sent);

		if (bytesWritten > 0) {
			client->getCgiContext().inputSent += bytesWritten;
			std::cout << "[CGI] Wrote " << bytesWritten << " bytes to the Python stdin pipe" << std::endl;
		} else if (bytesWritten == 0) {
			return;
		} else if (bytesWritten < 0) {
			epoll_ctl(_epollFd, EPOLL_CTL_DEL, pipeFd, NULL);
			close(pipeFd);
			_cgiWritePipes.erase(pipeFd);
			client->getCgiContext().stdin_fd = -1;
			if (client->getCgiContext().pid != -1) {
				kill(client->getCgiContext().pid, SIGKILL);
				reapChild(client->getCgiContext().pid);
				client->getCgiContext().pid = -1;
			}
			client->setResponse(ErrorBuilder::build(500, client->getCgiContext().errorPage500));
			client->setState(Client::WRITING);
			struct epoll_event event;
			event.events = EPOLLOUT;
			event.data.fd = client->getFd();
			epoll_ctl(_epollFd, EPOLL_CTL_MOD, client->getFd(), &event);
			return;
		}
	}

	if (client->getCgiContext().inputSent >= body.length()){
		std::cout << "[CGI] Request body sent. Sending EOF to stdin pipe." << std::endl;

		epoll_ctl(_epollFd, EPOLL_CTL_DEL, pipeFd, NULL);
		_cgiWritePipes.erase(pipeFd);

		close(pipeFd);
		client->getCgiContext().stdin_fd = -1;
	}
}

void Server::handleCgiOutput(int pipeFd) {
	Client* client = _cgiPipes[pipeFd];
	char buffer[4096];

	int bytesRead = read(pipeFd, buffer, sizeof(buffer) - 1);
	if (bytesRead > 0) {
		buffer[bytesRead] = '\0';
		client->getCgiContext().cgiOutput.append(buffer, bytesRead);
	}
	else if (bytesRead == 0) {
		std::cout << "[CGI] Python process finished. Building final response..." << std::endl;
		
		epoll_ctl(_epollFd, EPOLL_CTL_DEL, pipeFd, NULL);
		close(pipeFd);
		_cgiPipes.erase(pipeFd);

		CgiHandler::parseCgiOutput(client->getCgiContext().cgiOutput, *client);
		if (client->getCgiContext().pid != -1) {
			reapChild(client->getCgiContext().pid);
			client->getCgiContext().pid = -1;
		}
		
		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = client->getFd();
		epoll_ctl(_epollFd, EPOLL_CTL_MOD, client->getFd(), &event);
	}
	else if (bytesRead < 0) {
		epoll_ctl(_epollFd, EPOLL_CTL_DEL, pipeFd, NULL);
		close(pipeFd);
		_cgiPipes.erase(pipeFd);
		client->getCgiContext().stdout_fd = -1;
		if (client->getCgiContext().pid != -1) {
			kill(client->getCgiContext().pid, SIGKILL);
			reapChild(client->getCgiContext().pid);
			client->getCgiContext().pid = -1;
		}
		client->setResponse(ErrorBuilder::build(500, client->getCgiContext().errorPage500));
		client->setState(Client::WRITING);
		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = client->getFd();
		epoll_ctl(_epollFd, EPOLL_CTL_MOD, client->getFd(), &event);
	}
}