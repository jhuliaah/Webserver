/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:01:40 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/16 21:00:51 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "WebServer.hpp"

class Server{
	private:
		std::vector<Socket*>		_sockets;
		std::vector<int>			_serverFds;
		int							_epollFd;
		std::map<int, std::string>	_clientResponses;

		void	handleNewConnection(int server_fd);
		bool	readFromClient(int fd);
		void	writeToClient(int fd);
		bool	isServerFd(int fd);

	public:
		Server();
		~Server();
		void	initServer(std::vector<int> ports);
		void	serverLoop();
};