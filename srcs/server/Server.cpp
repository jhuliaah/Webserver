/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:06:07 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/23 15:29:14 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/WebServer.hpp"

//================================
//			Constructors		//
//================================

Server::Server() {
	_epollFd = epoll_create(10);
	if (_epollFd == -1)
		throw ServerException(std::string("epoll_create failed -> ") + strerror(errno));
}

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

void	Server::initServer(std::vector<int> ports){
	for (size_t i = 0; i < ports.size(); i++){
		Socket*	newSocket = new Socket();
		int		fd = newSocket->getFd();
	
		struct	sockaddr_in address;
		std::memset(&address, 0, sizeof(address));
		address.sin_family		= AF_INET; // usando IPv4
		address.sin_port		= htons(ports[i]); // htons é um tradutor (portavai ser 8080)
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

		std::cout << "Server listening on port " << ports[i] << " (fd: " << fd << ")" << std::endl;
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
    std::cout << "New client connected on fd: " << clientFd << std::endl;
}

void Server::removeClient(int fd) {
    epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    delete _clients[fd];
    _clients.erase(fd);
}

void Server::serverLoop(){
    const int MAX_EVENTS = 64;
    struct epoll_event events[MAX_EVENTS];

    while (true){
        int numEvents = epoll_wait(_epollFd, events, MAX_EVENTS, 2000);
        if(numEvents == -1) continue;
        
        for (int i = 0; i < numEvents; i++){
            int currentFd = events[i].data.fd;

            if (events[i].events & EPOLLIN){
                if (isServerFd(currentFd)){
                    handleNewConnection(currentFd);
                } else {
                    // Manda o Cliente ler os próprios dados
                    Client* client = _clients[currentFd];
                    if (client->readData() == true) {
                        
                        if (client->getState() == Client::CLOSED) {
                            removeClient(currentFd);
                        } 
                        else if (client->getState() == Client::WRITING) {
                            // O Cliente terminou de ler, muda para EPOLLOUT
                            struct epoll_event event;
                            event.events = EPOLLOUT;
                            event.data.fd = currentFd;
                            epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentFd, &event);
                        }
                    }
                }
            }
            else if (events[i].events & EPOLLOUT){
                // Manda o Cliente enviar os próprios dados
                Client* client = _clients[currentFd];
                if (client->writeData() == true) {
                    removeClient(currentFd);
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
        
        if (client->isTimeout(now, 60)) { // 60 segundos
            std::cout << "[CEIFADOR] Cliente no fd " << it->first << " expulso!" << std::endl;
            
            int fdToErase = it->first;
            ++it;
            removeClient(fdToErase);
        } else {
            ++it;
        }
    }
}
