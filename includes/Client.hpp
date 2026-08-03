/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 12:16:26 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/23 15:29:56 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "WebServer.hpp"

class Client {
	public:
		enum State {READING, WRITING, CLOSED};

	private:
		int			_fd;
		time_t		_lastActivity;
		std::string	_rawRequest;
		std::string	_response;
		State		_state;

		std::string _buildStaticResponse();
	public:

		Client(int fd);
		~Client();

		bool readData();
		bool writeData();
		bool isTimeout(time_t currentTime, int timeoutLimit);

		int		getFd() const { return _fd; }
		State	getState() const { return _state; }
};