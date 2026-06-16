/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:06:07 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/16 19:26:04 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/WebServer.hpp"

//================================
//			Constructors		//
//================================

Server::Server() {}

Server::~Server() {
	for (size_t i = 0; i < _sockets.size(); i++){
		close(_sockets[i]->getFd());
		delete _sockets[i];
	}
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
	
		struct pollfd serverPfd;
		serverPfd.fd		= fd;
		serverPfd.events	= POLLIN;
		serverPfd.revents	= 0;
		_fds.push_back(serverPfd);

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

	struct pollfd clientPdf;
	clientPdf.fd = clientFd;
	clientPdf.events = POLLIN;
	clientPdf.revents = 0;

	_fds.push_back(clientPdf);
	std::cout << "New client connected on fd: " << clientFd << std::endl;
}

void Server::writeToClient(size_t index){
	int fd = _fds[index].fd;
	std::string response = _clientResponses[fd];

	send(fd, response.c_str(), response.size(), 0);
	close(fd);

	_clientResponses.erase(fd);
	_fds.erase(_fds.begin() + index);
}

bool	Server::readFromClient(size_t index){
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));

	int bytes = recv(_fds[index].fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes > 0){
		std::string response = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 6\r\n\r\nShrek!";
		_clientResponses[_fds[index].fd] = response;
		_fds[index].events = POLLOUT;
		return false;
	}
	else {
		std::cout << "Client disconnected on fd: " << _fds[index].fd << std::endl;
		
		close(_fds[index].fd);
		_fds.erase(_fds.begin() + index);
		return true;
	}
}

void	Server::serverLoop(){
	while (true){
		poll(&_fds[0], _fds.size(), -1);
		for (size_t i = 0; i < _fds.size(); i++){

			if (_fds[i].revents & POLLIN){
				if (isServerFd(_fds[i].fd)){
					handleNewConnection(_fds[i].fd);
				} else {
					bool clientDeleted = readFromClient(i);
					if (clientDeleted == true){
						--i;
						continue;
					}
				}
			}
			if (_fds[i].revents & POLLOUT){
				writeToClient(i);
				i--;
			}
		}
	}
}