
#include "../../includes/Socket.hpp"
#include "../../includes/Exeptions.hpp"

Socket::Socket(){
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1)
		throw SocketException("ServerFd failed -> " + std::string(strerror(errno)));
	SocketConfig();
	nonBlocking();
};

void	Socket::SocketConfig(){
	int	opt = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw SocketException("Configuration failed -> " + std::string(strerror(errno)));
}

void	Socket::nonBlocking(){
	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1)
		throw SocketException("NonBlocking failed -> " + std::string(strerror(errno)));
	if (fcntl(_serverFd, F_SETFD, FD_CLOEXEC) == -1)
		throw SocketException("FD_CLOEXEC failed -> " + std::string(strerror(errno)));
}
