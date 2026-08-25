/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:06:07 by ratanaka          #+#    #+#             */
/*   Updated: 2026/08/14 15:41:27 by ratanaka         ###   ########.fr       */
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

#include <cerrno>			// errno
#include <cstring>			// strerror, memset
#include <ctime>			// time, time_t
#include <iostream>			// cout, endl
#include <signal.h>
#include <sys/wait.h>
#include "../../includes/CgiHandler.hpp"

/* 
O Router vai ser usado MAIS OU MENOS assim em Server.cpp, como utility class:
LocationConfig loc = Router::matchLoc(ServerConfig, uri);
RouteType type = Router::classify(loc, path, method);
IRequestHandler *handler = makeHandler(type);
bool done = handler->handle(req, *loc, client);
delete handler;																*/

//================================
//			Constructors		//
//================================

Server::Server(Config config) : _epollFd(-1), _config(config) {}

Server::~Server() {
	for (size_t i = 0; i < _sockets.size(); i++){
		close(_sockets[i]->getFd());
		delete _sockets[i];
	}
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
    _epollFd = epoll_create(10);
	if (_epollFd == -1)
		throw ServerException(std::string("epoll_create failed -> ") + strerror(errno));

	std::vector<ServerConfig> _servers = _config.getServers();
	for (size_t i = 0; i < _servers.size(); i++){
		int		currentPort = _servers[i].getPort();
		Socket*	newSocket = new Socket();
		int		fd = newSocket->getFd();
	
		struct	sockaddr_in address;
		std::memset(&address, 0, sizeof(address));
		address.sin_family		= AF_INET; // usando IPv4
		address.sin_port		= htons(currentPort); // htons é um tradutor (portavai ser 8080)
		address.sin_addr.s_addr	= inet_addr("127.0.0.1"); //por enquanto só vai aceitar conexões do localhost, mas futuramente vamos aceitar de qualquer lugar (INADDR_ANY)
	
		//bind() -> este socket vai receber conexoes na porta 8080
		if (bind(fd, (struct sockaddr*)&address, sizeof(address)) == -1){
			throw ServerException(std::string("bind() system call failed -> ") + strerror(errno)); }
		if (listen(fd, 10) == -1){
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

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = clientFd;
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientFd, &event) == -1){
        close(clientFd);
        return;
    }
    
    _clients[clientFd] = new Client(clientFd);
    _clients[clientFd]->setServerFd(serverFd);
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

void Server::removeClient(int fd) {
	if (_clients.find(fd) == _clients.end()) return;

	Client* client = _clients[fd];
	if (client->getCgiContext().pid != -1 || client->getCgiContext().stdout_fd != -1){
		int pipeFd = client->getCgiContext().stdout_fd;
		if (pipeFd != -1) {
			epoll_ctl(_epollFd, EPOLL_CTL_DEL, pipeFd, NULL);
			close(pipeFd);
			_cgiPipes.erase(pipeFd);
		}
		if (client->getCgiContext().pid != -1) {
			kill(client->getCgiContext().pid, SIGKILL);
			waitpid(client->getCgiContext().pid, NULL, WNOHANG);
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

void Server::dispatchRequest(int currentFd, Client* client)
{
	HttpParser parser;
	HttpParser::State parseState = parser.parse(client->getRawRequest(), client->getRequest());

	if (parseState == HttpParser::ERROR)
	{
		// request line ou headers malformados -> 400, nem tenta rotear
		client->setResponse(ErrorBuilder::build(400, ""));
		client->setState(Client::WRITING);

		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = currentFd;
		epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentFd, &event);
		return;
	}

	const ServerConfig& serverConfig = findServerConfig(client->getServerFd());
	const std::string& method = client->getRequest().getMethod();
	const std::string& uri = client->getRequest().getUri();

	LocationConfig loc = Router::matchLoc(serverConfig, uri);
	std::string filePath = resolveFilePath(loc, serverConfig, uri);
	RouteType type = Router::classify(loc, filePath, method);

	bool readyToWrite = false;

	if (type == ERROR)
	{
		// método não permitido na location -> 405
		std::string customErrorPage = "";
		const std::map<int, std::string>& errorPages = loc.getErrorPages();
		if (errorPages.find(405) != errorPages.end())
			customErrorPage = errorPages.find(405)->second;
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

    while (true){
        int numEvents = epoll_wait(_epollFd, events, MAX_EVENTS, 2000);
        if(numEvents == -1) continue;
        
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
						removeClient(currentFd);
					}
				}
            }
        }
        checkTimeouts();
    }
}

void Server::checkTimeouts() {
    time_t now = time(NULL);
    std::map<int, Client*>::iterator it = _clients.begin();
    
    while (it != _clients.end()) {
        Client* client = it->second;
		int clientFd = it->first;
        
		if (client->getState() == Client::CGI_RUNNING) {
			if (now - client->getCgiContext().startTime > 5) {
				std::cout << "[TIMEOUT 504] CGI script on fd " << clientFd << " timed out. Killing process!" << std::endl;
				if (client->getCgiContext().pid != -1){
					kill(client->getCgiContext().pid, SIGKILL);
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
				
				std::string error504 = "HTTP/1.1 504 Gateway Timeout\r\nContent-Type: text/html\r\nContent-Length: 47\r\n\r\n<html><body><h1>504 Gateway Timeout</h1></body></html>";
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
		} else if (bytesWritten < 0){ return; }
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
		
		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = client->getFd();
		epoll_ctl(_epollFd, EPOLL_CTL_MOD, client->getFd(), &event);
	}
}