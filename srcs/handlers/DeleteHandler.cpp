/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/03 17:52:46 by ratanaka          #+#    #+#             */
/*   Updated: 2026/08/13 20:43:30 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/DeleteHandler.hpp"
#include <unistd.h>
#include <iostream>
#include <sstream>

DeleteHandler::DeleteHandler() {}
DeleteHandler::~DeleteHandler() {}
		
bool DeleteHandler::handle(const HttpRequest& req, const LocationConfig& loc, Client& client){
	// Use location root if provided, otherwise default to ./www
	std::string root = loc.getRoot().empty() ? "./www" : loc.getRoot();
	std::string uri = req.getUri();
	if (!root.empty() && root[root.size() - 1] == '/')
		root.erase(root.size() - 1);
	if (uri.empty())
		uri = "/";
	std::string filePath = root + (uri[0] == '/' ? uri : std::string("/") + uri);
	std::string finalResponse;
	std::ostringstream responseStream;

	// Verificar se o arquivo existe
	if (access(filePath.c_str(), F_OK) == -1) {
		std::cout << "[DELETE] File not found: " << filePath << std::endl;
		std::string body = "<html><body><center><h1>404 Not Found</h1></center></body></html>";
		responseStream << "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " << body.length() << "\r\n\r\n" << body;
	}
	// Verificar se pode apagar
	else if (access(filePath.c_str(), W_OK) == -1) {
		std::cout << "[DELETE] No permission to delete: " << filePath << std::endl;
		std::string body = "<html><body><center><h1>403 Forbidden</h1></center></body></html>";
		responseStream << "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\nContent-Length: " << body.length() << "\r\n\r\n" << body;
	}
	else {
		if (unlink(filePath.c_str()) == 0) {
			std::cout << "[DELETE] File deleted successfully: " << filePath << std::endl;
			responseStream << "HTTP/1.1 204 No Content\r\n\r\n";
		} else {
			std::cout << "[DELETE] Internal error while deleting file." << std::endl;
			std::string body = "<html><body><center><h1>500 Internal Server Error</h1></center></body></html>";
			responseStream << "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nContent-Length: " << body.length() << "\r\n\r\n" << body;
		}
	}
	finalResponse = responseStream.str();
	client.setResponse(finalResponse);
	client.setState(Client::WRITING);
	return true;
}