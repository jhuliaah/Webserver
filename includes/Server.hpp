/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduribei <eduribei@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 14:01:40 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/20 20:19:54 by eduribei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "WebServer.hpp"

class Server
{
	private:
		std::vector<struct pollfd>	_fds;
		std::map<int, std::string>	_clientResponses;

		Socket	_socket;
		void	handleNewConnection();
		bool	readFromClient(size_t index);
		void	writeToClient(size_t index);

	public:
		void	initServer();
		void	serverLoop();
};
