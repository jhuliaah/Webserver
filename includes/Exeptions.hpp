/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exeptions.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/27 16:12:57 by ratanaka          #+#    #+#             */
/*   Updated: 2026/05/27 16:25:27 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "WebServer.hpp"

class ServerException : public std::exception {
	private :
		std::string _msg;
	public :
		ServerException(const std::string& error_detail) :_msg("Server Error: " + error_detail) {}
		virtual ~ServerException() throw() {}
		virtual const char* what() const throw() {return _msg.c_str();}
};

class SocketException : public std::exception {
	private:
		std::string _msg;
	public:
		SocketException(const std::string& detail) : _msg("Socket Error: " + detail) {}
		virtual ~SocketException() throw() {}
		virtual const char* what() const throw() { return _msg.c_str(); }
};