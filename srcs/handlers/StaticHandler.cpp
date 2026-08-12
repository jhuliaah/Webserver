/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 17:27:47 by ratanaka          #+#    #+#             */
/*   Updated: 2026/08/12 18:11:17 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/StaticHandler.hpp"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <sstream>

StaticHandler::StaticHandler() {}

StaticHandler::~StaticHandler() {}

bool StaticHandler::handle(const HttpRequest& req, const LocationConfig& loc, Client& client){
	std::string root = loc.getRoot();
	
	if (root.empty()){root = "./www";}
	std::string filePath = root + req.getUri();
	
	struct stat			path_stat;
	std::stringstream	responseStream;
	std::string			body;
	
	// O Ficheiro/Pasta existe?
	if (stat(filePath.c_str(), &path_stat) != 0){
		std::cout << "[STATIC] Erro 404: Não encontrado -> " << filePath << std::endl;
		body = "<html><body><center><h1>404 Not Found</h1></center></body></html>";
		responseStream << "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " << body.length() << "\r\n\r\n" << body;
	}
	// pasta comum
	else if (S_ISDIR(path_stat.st_mode)){
		std::string indexFile = loc.getIndex();
		
		if(!indexFile.empty()) {
			std::cout << "[STATIC] Redirecionando para o index configurado: " << indexFile << std::endl;
			filePath += (filePath[filePath.length() - 1] == '/' ? "" : "/") + indexFile;
			body = "<html><body><h1>Index carregado com sucesso do Config!</h1></body></html>";
			responseStream << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " << body.length() << "\r\n\r\n" << body;
		}
		else if (loc.getAutoindex() == true) {
			std::cout << "[STATIC] Autoindex LIGADO! Listando pasta -> " << filePath << std::endl;
			body = "<html><body><h1>Aqui vai nascer o Autoindex dinâmico!</h1></body></html>";
			responseStream << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " << body.length() << "\r\n\r\n" << body;
		}
		else {
			std::cout << "[STATIC] Erro 403: Autoindex desligado -> " << filePath << std::endl;
			body = "<html><body><center><h1>403 Forbidden</h1></center></body></html>";
			responseStream << "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\nContent-Length: " << body.length() << "\r\n\r\n" << body;
		}
	}
	// ficheiro comum
	else {
		std::cout << "[STATIC] Lendo ficheiro normal -> " << filePath << std::endl;
		std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
		if (file.is_open()) {
			std::stringstream buffer;
			buffer << file.rdbuf();
			body = buffer.str();
			file.close();
			
			responseStream << "HTTP/1.1 200 OK\r\n";
			responseStream << "Content-Type: text/html\r\n";
			responseStream << "Content-Length: " << body.length() << "\r\n\r\n";
			responseStream << body;
		} else {
			std::cout << "[STATIC] Erro 403: Sem permissão de leitura -> " << filePath << std::endl;
			body = "<html><body><center><h1>403 Forbidden</h1></center></body></html>";
			responseStream << "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\nContent-Length: " << body.length() << "\r\n\r\n" << body;
		}
	}
	client.setResponse(responseStream.str());
	client.setState(Client::WRITING);
	return true;
}


