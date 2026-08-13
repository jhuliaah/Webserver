/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 17:27:47 by ratanaka          #+#    #+#             */
/*   Updated: 2026/08/13 17:21:08 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/StaticHandler.hpp"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
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
			body = buildAutoIndex(filePath, req.getUri());
			responseStream << "HTTP/1.1 200 OK\r\n"
							<< "Content-Type: text/html\r\n"
							<< "Content-Length: " << body.length() << "\r\n\r\n" 
							<< body;
		}
		else {
			std::cout << "[STATIC] Erro 403: Autoindex desligado -> " << filePath << std::endl;
			body = "<html><body><center><h1>403 Forbidden</h1></center></body></html>";
			responseStream << "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\nContent-Length: " << body.length() << "\r\n\r\n" << body;
		}
	}
	// ficheiro comum
	else {
		std::cout << "[STATIC] A iniciar Stream por Chunks para -> " << filePath << " (" << path_stat.st_size << " bytes)" << std::endl;
		responseStream << "HTTP/1.1 200 OK\r\n";
		responseStream << "Content-Type: " << getMimeType(filePath) << "\r\n";
		responseStream << "Content-Length: " << path_stat.st_size << "\r\n";
		responseStream << "Connection: keep-alive\r\n\r\n";
		if (client.startFileStream(filePath, path_stat.st_size) == true) {
			client.setResponse(responseStream.str());
			client.setState(Client::WRITING);
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

std::string StaticHandler::buildAutoIndex(const std::string& path, const std::string& uri){
	std::stringstream	html;
	DIR					*dir;
	struct dirent		*ent;

	std::string safeUri = uri;
	if (!safeUri.empty() && safeUri[safeUri.length() - 1] != '/') {
		safeUri += "/";
	}

	html << "<!DOCTYPE html>\n<html>\n<head>\n<title>Index of " << safeUri << "</title>\n</head>\n<body>\n";
	html << "<h1>Index of " << safeUri << "</h1>\n<hr>\n<pre>\n";

	if ((dir = opendir(path.c_str())) != NULL){
		while ((ent = readdir(dir)) != NULL) {
			std::string filename = ent->d_name;
			html << "<a href=\"" << safeUri << filename << "\">" << filename << "</a><br>\n";
		}
		closedir(dir);
	} else {
		html << "Erro: Nao foi possivel abrir o diretorio.\n";
	}
	html << "</pre>\n<hr>\n</body>\n</html>\n";
	return html.str();
}

std::string StaticHandler::getMimeType(const std::string& path){
	size_t dotPos = path.find_last_of(".");
	if (dotPos == std::string::npos) {return "text/plain";}
	
	std::string ext = path.substr(dotPos);
	if (ext == ".html" || ext == ".htm") return "text/html";
	if (ext == ".css")  return "text/css";
	if (ext == ".js")   return "text/javascript";
	if (ext == ".png")  return "image/png";
	if (ext == ".jpg"  || ext == ".jpeg") return "image/jpeg";
	if (ext == ".gif")  return "image/gif";
	if (ext == ".ico")  return "image/x-icon";
	if (ext == ".mp4")  return "video/mp4";
	if (ext == ".pdf")  return "application/pdf";
	if (ext == ".json") return "application/json";

	return "text/plain";
}