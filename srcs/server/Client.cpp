/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 12:24:50 by ratanaka          #+#    #+#             */
/*   Updated: 2026/08/13 17:34:17 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Client.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>

/*Rafael, não sei onde, mas seu client em algum momento vai precisar checar
o timeout do CGI a partir de CgiContext.startTime, uma struct nova de Client.*/

Client::Client(int fd) : _fd(fd), _state_e(READING), _contentLength(-1) {
    _lastActivity = time(NULL);
}

Client::~Client() {}

bool Client::isTimeout(time_t currentTime, int timeoutLimit) {
    return (currentTime - _lastActivity > timeoutLimit);
}

bool Client::readData() {

    char buffer[4096];
    std::memset(buffer, 0, sizeof(buffer));

    int bytes = recv(_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes > 0)
    {
		_lastActivity = time(NULL);
		_rawRequest.append(buffer, bytes);

		size_t headerEnd = _rawRequest.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
			return (false);

		if (_contentLength == -1)
		{
			size_t C1Pos = _rawRequest.find("Content-Length:");
			if (C1Pos != std::string::npos && C1Pos < headerEnd)
		_contentLength = std::atol(_rawRequest.c_str() + C1Pos + 16);
			else
		_contentLength = 0;
		}

		size_t bodyReceived = _rawRequest.size() - (headerEnd + 4);

		if (static_cast<long>(bodyReceived) < _contentLength)
			return (false);

		std::cout << "Complete request received on fd: " << _fd << std::endl;
		return (true);
    }
    else{
		_state_e = CLOSED;
		return (true);
    }

}

bool Client::writeData() {
	if (!_response.empty()){
		ssize_t sent = send(_fd, _response.c_str(), _response.length(), 0);
		if (sent > 0) {_response.erase(0, sent);}
		else if (sent < 0) {return false;}
	}
	if (_response.empty() && _isStreamingFile) {return sendNextChunk();}
	return _response.empty() && !_isStreamingFile;
}

bool Client::startFileStream(const std::string& filePath, size_t fileSize){
	if (_fileStream.is_open()) {_fileStream.close();}
	_fileStream.open(filePath.c_str(), std::ios::in | std::ios::binary);
	if (!_fileStream.is_open()) {return false;}
	_fileSize = fileSize;
	_fileBytesSent = 0;
	_isStreamingFile = true;
	return true;
}

bool Client::isStreamingFile() const{ return _isStreamingFile; }

void Client::closeFileStream(){
	if (_fileStream.is_open()) { _fileStream.close(); }
	_fileSize = 0; _fileBytesSent = 0; _isStreamingFile = false;
}

bool Client::sendNextChunk(){
	if (!_isStreamingFile || !_fileStream.is_open()) { return true; }
	
	char buffer[8192];
	_fileStream.read(buffer, sizeof(buffer));
	std::streamsize bytesRead = _fileStream.gcount();

	if (bytesRead > 0) {
		ssize_t bytesSent = send(_fd, buffer, bytesRead, 0);
		if (bytesSent > 0) {_fileBytesSent += bytesSent;}
		else if (bytesSent < 0) {return false;}
	}
	if (_fileStream.eof() || _fileBytesSent >= _fileSize) {
		closeFileStream(); return true;
	}
	return false;
}

