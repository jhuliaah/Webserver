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

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <cstddef>
#include <ctime>
#include <sys/types.h>

class Client {
	public:
		enum State {READING, WRITING, CGI_RUNNING, CLOSED};
		struct CgiContext {
			pid_t		pid;
			int			stdin_fd;
			int			stdout_fd;
			std::string outputBuffer;
			size_t		inputSent;
			time_t		startTime;
		} _cgiContext; // copiei CgiContext de vados-sa

	private:
		int			_fd;
		time_t		_lastActivity;
		std::string	_rawRequest;
		std::string	_response;
		State		_state_e;
		//lembranças do norminette: typedef = _t, struct = _s, enum = _e

		std::string _buildStaticResponse();

	public:
		Client(int fd);
		~Client();

		bool readData();
		bool writeData();
		bool isTimeout(time_t currentTime, int timeoutLimit);

		int		getFd() const { return _fd; }
		State	getState() const { return _state_e; }


};

#endif



