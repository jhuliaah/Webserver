/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:01:40 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/03 17:50:36 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "WebServer.hpp"

class Server : public Socket {
	private:
		std::vector<struct pollfd> _fds;
		std::map<int, std::string> _clientResponses;

		void	handleNewConnection();
		bool	readFromClient(size_t index);
		void	writeToClient(size_t index);

	public:
		void	initServer();
		void	serverLoop();
};