/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:06:07 by ratanaka          #+#    #+#             */
/*   Updated: 2026/05/28 15:15:04 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/WebServer.hpp"

void	Server::initServer(){

	struct sockaddr_in address;

	std::memset(&address, 0, sizeof(address));

	address.sin_family		= AF_INET; // usando IPv4
	address.sin_port		= htons(8080); // htons é um tradutor (portavai ser 8080)
	address.sin_addr.s_addr	= INADDR_ANY; //qualquer ip ou interface configurada é aceita

	//bind() -> este socket vai receber conexoes na porta 8080
	if (bind(_serverFd, (struct sockaddr*)&address, sizeof(address)) == -1){
		throw ServerException(std::string("bind() system call failed -> ") + strerror(errno)); }
	if (listen(_serverFd, 10) == -1){
		throw ServerException(std::string("listen() system call failed -> ") + strerror(errno));}
}

void	Server::handleNewConnection(){
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);

	int clientFd = accept(_serverFd, (struct sockaddr*)&clientAddr, &clientLen);
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

void	Server::handleClientData(size_t index){
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));

	int bytes = recv(_fds[index].fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes > 0){
		std::cout << "Received data: " << buffer << std::endl;

		std::string response = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 6\r\n\r\nShrek!";
		send(_fds[index].fd, response.c_str(), response.size(), 0);

		close(_fds[index].fd);
		_fds.erase(_fds.begin() + index);
	}
	else {
		std::cout << "Client disconnected on fd: " << _fds[index].fd << std::endl;
		
		close(_fds[index].fd);
		_fds.erase(_fds.begin() + index);
	}
}

void	Server::serverLoop(){
	struct pollfd _serverPfd;

	_serverPfd.fd		= _serverFd;
	_serverPfd.events	= POLLIN;
	_serverPfd.revents	= 0;

	_fds.push_back(_serverPfd);

	while (true){
		poll(&_fds[0], _fds.size(), -1);
		for (size_t i = 0; i < _fds.size(); i++){
			if (_fds[i].revents & POLLIN){
				handleNewConnection();
				} else {
					handleClientData(i);
					--i;
			}
		}
	}
}