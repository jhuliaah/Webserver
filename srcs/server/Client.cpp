/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 12:24:50 by ratanaka          #+#    #+#             */
/*   Updated: 2026/09/03 13:55:07 by ratanaka         ###   ########.fr       */
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

static bool decodeChunkedBody(const std::string& raw, size_t bodyStart,
	std::string& decoded)
{
	size_t position = bodyStart;
	decoded.clear();

	while (true)
	{
		size_t lineEnd = raw.find("\r\n", position);
		if (lineEnd == std::string::npos)
			return false;
		std::string sizeLine = raw.substr(position, lineEnd - position);
		size_t extension = sizeLine.find(';');
		if (extension != std::string::npos)
			sizeLine.erase(extension);
		char* end = NULL;
		long chunkSize = std::strtol(sizeLine.c_str(), &end, 16);
		if (end == sizeLine.c_str() || *end != '\0' || chunkSize < 0)
			return false;
		position = lineEnd + 2;

		if (raw.size() < position + static_cast<size_t>(chunkSize) + 2)
			return false;
		decoded.append(raw, position, static_cast<size_t>(chunkSize));
		position += static_cast<size_t>(chunkSize);
		if (raw.compare(position, 2, "\r\n") != 0)
			return false;
		position += 2;
		if (chunkSize == 0)
			return true;
	}
}

static std::string headerValue(const std::string& raw, size_t headerEnd,
	const std::string& wanted)
{
	size_t lineStart = 0;
	while (lineStart < headerEnd)
	{
		size_t lineEnd = raw.find("\r\n", lineStart);
		if (lineEnd == std::string::npos || lineEnd > headerEnd)
			break;
		size_t colon = raw.find(':', lineStart);
		if (colon != std::string::npos && colon < lineEnd)
		{
			std::string name = raw.substr(lineStart, colon - lineStart);
			for (size_t i = 0; i < name.size(); ++i)
				name[i] = static_cast<char>(std::tolower(name[i]));
			if (name == wanted)
				return raw.substr(colon + 1, lineEnd - colon - 1);
		}
		lineStart = lineEnd + 2;
	}
	return "";
}

/*Rafael, não sei onde, mas seu client em algum momento vai precisar checar
o timeout do CGI a partir de CgiContext.startTime, uma struct nova de Client.*/

Client::Client(int fd) : _fd(fd), _serverFd(-1), _state_e(READING), _contentLength(-1),
	_fileSize(0), _fileBytesSent(0), _isStreamingFile(false) {
    _lastActivity = time(NULL);
}

Client::~Client() {}

void Client::prepareNextRequest() {
	_rawRequest.clear();
	_request = HttpRequest();
	_contentLength = -1;
	_state_e = READING;
	_lastActivity = time(NULL);
	_cgiContext.cgiOutput.clear();
	_cgiContext.inputSent = 0;
}

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
			std::string transfer = headerValue(_rawRequest, headerEnd, "transfer-encoding");
			std::string length = headerValue(_rawRequest, headerEnd, "content-length");
			for (size_t i = 0; i < transfer.size(); ++i)
				transfer[i] = static_cast<char>(std::tolower(transfer[i]));
			if (transfer.find("chunked") != std::string::npos)
				_contentLength = -2;
			else if (!length.empty())
				_contentLength = std::atol(length.c_str());
			else
		_contentLength = 0;
		}

		size_t bodyReceived = _rawRequest.size() - (headerEnd + 4);
		if (_contentLength == -2)
		{
			std::string decodedBody;
			if (!decodeChunkedBody(_rawRequest, headerEnd + 4, decodedBody))
				return false;
			std::string headers = _rawRequest.substr(0, headerEnd + 4);
			_rawRequest = headers + decodedBody;
			_contentLength = static_cast<long>(decodedBody.size());
			bodyReceived = decodedBody.size();
		}

		if (static_cast<long>(bodyReceived) < _contentLength)
			return (false);

		std::cout << "Complete request received on fd: " << _fd << std::endl;
		return (true);
    }
     else if (bytes == 0)
    {
		_state_e = CLOSED;
		return (true);
    }
	else if (bytes < 0)
	{
		_state_e = CLOSED;
		return (true);
	}
	return (false);
}

bool Client::writeData() {
	if (!_response.empty()){
		ssize_t sent = send(_fd, _response.c_str(), _response.length(), 0);
		if (sent > 0) {_response.erase(0, sent);}
		else if (sent == 0) return false;
		else if (sent < 0) {_state_e = CLOSED; return true;}
		return false;
	}
	if (_isStreamingFile) return sendNextChunk();
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
		std::string chunk(buffer, static_cast<size_t>(bytesRead));
		ssize_t bytesSent = send(_fd, chunk.c_str(), chunk.length(), 0);
		if (bytesSent > 0) {
			_fileBytesSent += bytesSent;
			if (static_cast<size_t>(bytesSent) < chunk.length())
				_response.assign(chunk, static_cast<size_t>(bytesSent), std::string::npos);
		}
		else if (bytesSent == 0)
			_response = chunk;
		else if (bytesSent < 0) {_state_e = CLOSED; return true;}
	}
	if (_fileStream.eof() && _response.empty() && _fileBytesSent >= _fileSize) {
		closeFileStream(); return true;
	}
	return false;
}

