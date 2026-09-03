/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:01:40 by ratanaka          #+#    #+#             */
/*   Updated: 2026/08/14 15:31:44 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <map>
#include <string>
#include <sys/types.h>

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
		// pids de CGI já mortos/finalizados (SIGKILL ou stdout fechado) mas
		// que um waitpid(..., WNOHANG) ainda não conseguiu colher na hora --
		// reapPendingChildren() tenta de novo a cada volta do serverLoop, em
		// vez de bloquear o loop inteiro esperando o processo terminar.
		std::vector<pid_t>			_pendingReap;
		Config						_config;


		void	handleNewConnection(int server_fd);
		bool	isServerFd(int fd);
		void	checkTimeouts();
		void	handleCgiOutput(int pipeFd);
		void	handleCgiWrite(int pipeFd);
		void	reapChild(pid_t pid);
		void	reapPendingChildren();

		void removeClient(int fd);

		void dispatchRequest(int currentFd, Client* client);
		const ServerConfig& findServerConfig(int serverFd) const;

		Server();				// virou private, so cria server com config.

	public:
		Server(Config config);
		~Server();
		void	initServer(); // os fake ports mudaram de lugar.
		void	serverLoop();
};

#endif