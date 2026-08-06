/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 12:24:50 by ratanaka          #+#    #+#             */
/*   Updated: 2026/08/06 15:37:08 by ratanaka         ###   ########.fr       */
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

Client::Client(int fd) : _fd(fd), _state_e(READING), _bytesSent(0), _contentLength(-1) {
    _lastActivity = time(NULL);
}

Client::~Client() {}

bool Client::isTimeout(time_t currentTime, int timeoutLimit) {
    return (currentTime - _lastActivity > timeoutLimit);
}

std::string Client::_buildStaticResponse() {
    std::ifstream   file("./www/index.html");
    std::string     finalResponse;

    if (file.is_open()){
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string body = buffer.str();
        file.close();

        std::stringstream responseStream;
        responseStream << "HTTP/1.1 200 OK\r\n";
        responseStream << "Content-Type: text/html; charset=utf-8\r\n";
        responseStream << "Content-Length: " << body.length() << "\r\n\r\n";
        responseStream << body;

        finalResponse = responseStream.str();
    } else {
        std::string errorBody = "<html><body><center><h1>404 Not Found</h1></center></body></html>";
        std::stringstream responseStream;
        responseStream << "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " << errorBody.length() << "\r\n\r\n" << errorBody;
        finalResponse = responseStream.str();
    }
    return finalResponse;
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

        std::cout << "Complete requisition received in: " <<_fd << " fd" << std::endl;
        _response = _buildStaticResponse();
        _state_e = WRITING;
        return (true);
    }
    else{
        _state_e = CLOSED;
        return (true);
    }

}

bool Client::writeData() {
    const char *data = _response.c_str() + _bytesSent;
    size_t remaining = _response.size() - _bytesSent;

    int bytesSent = send(_fd, data, remaining, 0);

    if (bytesSent > 0)
        _bytesSent += bytesSent;

    if (_bytesSent >= _response.size()) {
        _state_e = CLOSED; // mandou tudo, agora sim pode fechar
        return true;
    }

    return false; // ainda falta mandar, o Server chama de novo no próximo poll()
}
