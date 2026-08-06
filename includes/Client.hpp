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
#include "HttpRequest.hpp"

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

			CgiContext() : pid(-1), stdin_fd(-1), stdout_fd(-1), inputSent(false), startTime(0) {}
		};

	private:
		int				_fd;
		time_t			_lastActivity;
		std::string		_rawRequest;
		std::string		_response;
		State			_state_e;

		HttpRequest		_request;
		CgiContext		_cgiContext;
		size_t			_bytesSent;
		long			_contentLength;
		//norminette memories: typedef = _t, struct = _s, enum = _e

		std::string _buildStaticResponse();

	public:
		Client(int fd);
		~Client();

		bool readData();
		bool writeData();
		bool isTimeout(time_t currentTime, int timeoutLimit);

		int		getFd() const { return _fd; }
		State	getState() const { return _state_e; }
		void	setState(State state) {_state_e = state;}

		CgiContext& getCgiContext() {return _cgiContext;}
		HttpRequest& getRequest() {return _request;}
		std::string&	getRawRequest() {return _rawRequest;}

		void	setResponse(const std::string& res) { _response = res;}
};

#endif



