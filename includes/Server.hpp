
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

		Server();

	public:
		Server(Config config);
		~Server();
		void	initServer();
		void	serverLoop();
};

#endif