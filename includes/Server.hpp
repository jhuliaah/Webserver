/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:01:40 by ratanaka          #+#    #+#             */
<<<<<<< HEAD
/*   Updated: 2026/07/28 17:46:16 by ratanaka         ###   ########.fr       */
=======
/*   Updated: 2026/06/23 12:37:13 by ratanaka         ###   ########.fr       */
>>>>>>> 8b7d808a487617deda68f9be990da1e3ad802110
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <map>
#include <string>

#include "Socket.hpp"
#include "Config.hpp"

class Client;

class Server{
	private:
		int							_epollFd;
		std::vector<Socket*>		_sockets;
		std::vector<int>			_serverFds;
		std::map<int, Client*>		_clients;
		std::map<int, Client*>		_cgiPipes;
		std::map<int, Client*>		_cgiWritePipes;
		Config						_config;




		void	handleNewConnection(int server_fd);
		bool	isServerFd(int fd);
		void	checkTimeouts();
		void	handleCgiRead(int pipeFd);
		void	handleCgiWrite(int pipeFd);

		void removeClient(int fd);

		Server();				// virou private, so cria server com config.

	public:
		Server(Config config);
		~Server();
		void	initServer(); // os fake ports mudaram de lugar.
		void	serverLoop();
};

#endif