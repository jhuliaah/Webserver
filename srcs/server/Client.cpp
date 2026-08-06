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
            // _response = _buildStaticResponse();
            
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

	if (!_response.empty()) {
		int bytesSent = send(_fd, _response.c_str(), _response.size(), 0);
		if (bytesSent > 0) {
            std::cout << "-> Resposta enviada para o fd " << _fd << "!" << std::endl;
		}
		_response.clear();
        _state_e = CLOSED;
		return true;
	}

    _state_e = CLOSED;
    return true;
}