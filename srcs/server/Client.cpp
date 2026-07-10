/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 12:24:50 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/23 15:23:03 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Client.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>		

/*Rafael, não sei onde, mas seu client em algum momento vai precisar checar 
o timeout do CGI a partir de CgiContext.startTime, uma struct nova de Client.*/

Client::Client(int fd) : _fd(fd), _state_e(READING) {
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

    if (bytes > 0) {
        _lastActivity = time(NULL); // Atualiza o relógio!
        _rawRequest.append(buffer, bytes);
        
        if (_rawRequest.find("\r\n\r\n") != std::string::npos) {
            std::cout << "Requisicao completa recebida do fd: " << _fd << std::endl;
            
            // Aqui futuramente chamaremos o Parser
            _response = _buildStaticResponse();
            
            _state_e = WRITING; // Muda o estado do client
            return true; // Retorna true avisando o Server que quer escrever
        }
        return false; // Ainda não terminou de ler os pacotes
    }
    else {
        _state_e = CLOSED;
        return true; // O cliente desconectou ou deu erro
    }
}

bool Client::writeData() {
    //Pegamos a bandeja (_response) e atiramos pelo tubo do Socket (_fd)
    int bytesSent = send(_fd, _response.c_str(), _response.size(), 0);
    
    if (bytesSent > 0) {
        _state_e = CLOSED; // No HTTP/1.1 básico, terminamos e fechamos.
        return true; // Retorna true avisando o Server que pode matá-lo
    }
    
    _state_e = CLOSED;
    return true; // Erro no envio
}