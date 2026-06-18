/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:06:07 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/16 21:26:22 by ratanaka         ###   ########.fr       */
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
		address.sin_addr.s_addr	= INADDR_ANY; //qualquer ip ou interface configurada é aceita
	
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

void	Server::handleNewConnection(int serverFd){
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);

	int clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &clientLen);
	if (clientFd == -1)
		return;

	fcntl(clientFd, F_SETFL, O_NONBLOCK);

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = clientFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientFd, &event) == -1){
		close(clientFd);
		return;
	}

	std::cout << "New client connected on fd: " << clientFd << std::endl;
}

void Server::writeToClient(int fd){
	std::string response = _clientResponses[fd];

	send(fd, response.c_str(), response.size(), 0);

	epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);

	_clientResponses.erase(fd);
}

bool	Server::readFromClient(int fd){
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));

	int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes > 0){
		std::string response = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 6\r\n\r\nShrek!";
		_clientResponses[fd] = response;

		struct epoll_event event;
		event.events = EPOLLOUT;
		event.data.fd = fd;
		epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &event);

		return false;
	}
	else {
		std::cout << "Client disconnected on fd: " << fd << std::endl;
		epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		return true;
	}
}

void	Server::serverLoop(){
	const int MAX_EVENTS = 64;
	struct epoll_event events[MAX_EVENTS];

	while (true){
		int numEvents = epoll_wait(_epollFd, events, MAX_EVENTS, -1);
		if(numEvents == -1)
			continue;
		
		for (int i = 0; i < numEvents; i++){
			int currentFd = events[i].data.fd;

			if (events[i].events & EPOLLIN){
				if (isServerFd(currentFd)){
					handleNewConnection(currentFd);
				} else {
					readFromClient(currentFd);
				}
			}
			else if (events[i].events & EPOLLOUT){
				writeToClient(currentFd);
			}
		}
	}
}