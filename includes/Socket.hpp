
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <fcntl.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstring>

class Socket{
	protected :
		int	_serverFd;
	public :
		Socket();

		void	SocketConfig();
		void	nonBlocking();
		int		getFd() const {return _serverFd;};
};

#endif