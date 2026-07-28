/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 17:39:04 by ratanaka          #+#    #+#             */
/*   Updated: 2026/07/21 17:45:03 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include <iostream>
# include <string>

# include <sys/socket.h>	// socket, bind, listen, accept, send, recv
# include <cerrno>			// errno
# include <netinet/in.h>	// sockaddr_in, INADDR_ANY, htons
# include <fcntl.h>			// fcntl, O_NONBLOCK
# include <arpa/inet.h>

# include <poll.h>			// poll, struct pollfd
# include <sys/epoll.h>

# include <unistd.h>		// close, read, write
# include <cstring>			// memset, memcpy
# include <vector>
# include <map>

# include <sys/types.h>
# include <sys/wait.h>

# include <ctime>

# include <fstream>
# include <sstream>

# include "Socket.hpp"
# include "Server.hpp"
# include "Exeptions.hpp"
# include "Client.hpp"
# include "Config.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "IRequestHandler.hpp"
# include "LocationConfig.hpp"
# include "RouteConfig.hpp"
# include "Router.hpp"
# include "ServerConfig.hpp"
# include "StaticHandler.hpp"
# include "CgiHandler.hpp"
# include "CgiRequest.hpp"