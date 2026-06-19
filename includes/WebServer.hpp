/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 17:39:04 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/19 14:05:44 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <iostream>
# include <string>

# include <sys/socket.h>	// socket, bind, listen, accept, send, recv
# include <cerrno>			// errno
# include <netinet/in.h>	// sockaddr_in, INADDR_ANY, htons
# include <fcntl.h>			// fcntl, O_NONBLOCK

# include <poll.h>			// poll, struct pollfd
# include <sys/epoll.h>

# include <unistd.h>		// close, read, write
# include <cstring>			// memset, memcpy
# include <vector>
# include <map>

# include <fstream>
# include <sstream>

# include "Socket.hpp"
# include "Server.hpp"
# include "Exeptions.hpp"