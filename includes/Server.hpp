/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:01:40 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/23 12:37:13 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <map>
#include <string>

#include "Socket.hpp"
#include "Client.hpp"
#include "Config.hpp"

class Server{
	private:
		int							_epollFd;
		std::vector<Socket*>		_sockets;
		std::vector<int>			_serverFds;
		std::map<int, Client*>		_clients;
		std::map<int, Client*>		_cgiPipes;
		Config						_config;




		void	handleNewConnection(int server_fd);
		bool	readFromClient(int fd);
		void	writeToClient(int fd);
		bool	isServerFd(int fd);
		void	checkTimeouts();

		void removeClient(int fd);

		Server();
		std::string _buildStaticResponse();

	public:
		Server(Config config);
		~Server();
		void	initServer();
		void	serverLoop();
};

#endif