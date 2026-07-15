/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 17:34:33 by ratanaka          #+#    #+#             */
/*   Updated: 2026/05/27 14:18:25 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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