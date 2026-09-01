/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 17:27:47 by ratanaka          #+#    #+#             */
/*   Updated: 2026/09/01 15:32:58 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/StaticHandler.hpp"
#include "../../includes/ErrorBuilder.hpp"
#include "../../includes/HttpResponse.hpp"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sstream>
#include <map>

static std::string resolveCustomErrorPagePath(const LocationConfig& loc, const std::string& pagePath)
{
	if (pagePath.empty())
		return "";
	if (pagePath[0] == '/' || pagePath.compare(0, 2, "./") == 0)
		return pagePath;

	std::string root = loc.getRoot();
	if (root.empty())
		root = "./www";
	if (!root.empty() && root[root.size() - 1] == '/')
		root.erase(root.size() - 1);
	return root + "/" + pagePath;
}

StaticHandler::StaticHandler() {}

StaticHandler::~StaticHandler() {}

bool StaticHandler::handle(const HttpRequest& req, const LocationConfig& loc, Client& client){
	std::string root = loc.getRoot();
	bool isHead = (req.getMethod() == "HEAD");

	if (root.empty()){root = "./www";}
	if (root[root.size() - 1] == '/')
		root.erase(root.size() - 1);
	std::string filePath = root + req.getUri();

	struct stat path_stat;

	// O Ficheiro/Pasta existe?
	if (stat(filePath.c_str(), &path_stat) != 0){
		std::cout << "[STATIC] Error 404: not found -> " << filePath << std::endl;
		std::string customErrorPage = "";
		const std::map<int, std::string>& errorPages = loc.getErrorPages();
		if (errorPages.find(404) != errorPages.end()) {
			customErrorPage = resolveCustomErrorPagePath(loc, errorPages.find(404)->second);
		}
		client.setResponse(ErrorBuilder::build(404, customErrorPage));
		client.setState(Client::WRITING);
		return true;
	}
	// pasta comum
	else if (S_ISDIR(path_stat.st_mode)){
		std::string indexFile = loc.getIndex();
		std::string indexPath = filePath;
		if (!indexPath.empty() && indexPath[indexPath.length() - 1] != '/')
			indexPath += "/";
		indexPath += indexFile;

		struct stat indexStat;
		bool indexExists = (!indexFile.empty()
			&& stat(indexPath.c_str(), &indexStat) == 0
			&& S_ISREG(indexStat.st_mode));

		if (indexExists) {
			std::cout << "[STATIC] Serving configured index: " << indexPath << std::endl;

			if (client.startFileStream(indexPath, indexStat.st_size) == true) {
				HttpResponse res;
				res.status_code = 200;
				res.headers["Content-Type"] = getMimeType(indexPath);
				res.headers["Connection"] = "keep-alive";

				std::ostringstream lenStream;
				lenStream << indexStat.st_size;
				res.headers["Content-Length"] = lenStream.str();
				if (isHead)
					res.body.clear();

				client.setResponse(res.serialize());
				client.setState(Client::WRITING);
				return true;
			} else {
				std::cout << "[STATIC] Error 403: no read permission -> " << indexPath << std::endl;
				std::string customErrorPage = "";
				const std::map<int, std::string>& errorPages = loc.getErrorPages();
				if (errorPages.find(403) != errorPages.end()) {
					customErrorPage = resolveCustomErrorPagePath(loc, errorPages.find(403)->second);
				}
				client.setResponse(ErrorBuilder::build(403, customErrorPage));
				client.setState(Client::WRITING);
				return true;
			}
		}
		else if (loc.getAutoindex() == true) {
			std::cout << "[STATIC] Autoindex enabled. Listing directory -> " << filePath << std::endl;

			HttpResponse res;
			res.status_code = 200;
			res.headers["Content-Type"] = "text/html";
			std::string body = buildAutoIndex(filePath, req.getUri());
			if (isHead)
				body.clear();
			res.body = body;
			std::ostringstream lenStream;
			lenStream << body.size();
			res.headers["Content-Length"] = lenStream.str();
			client.setResponse(res.serialize());
			client.setState(Client::WRITING);
			return true;
		}
		else {
			std::cout << "[STATIC] Error 403: autoindex disabled -> " << filePath << std::endl;
			std::string customErrorPage = "";
			const std::map<int, std::string>& errorPages = loc.getErrorPages();
			if (errorPages.find(403) != errorPages.end()) {
				customErrorPage = resolveCustomErrorPagePath(loc, errorPages.find(403)->second);
			}
			client.setResponse(ErrorBuilder::build(403, customErrorPage));
			client.setState(Client::WRITING);
			return true;
		}
	}
	// ficheiro comum
	else {
		std::cout << "[STATIC] Starting chunked stream for -> " << filePath << " (" << path_stat.st_size << " bytes)" << std::endl;

		if (client.startFileStream(filePath, path_stat.st_size) == true) {
			HttpResponse res;
			res.status_code = 200;
			res.headers["Content-Type"] = getMimeType(filePath);
			res.headers["Connection"] = "keep-alive";

			std::ostringstream lenStream;
			lenStream << path_stat.st_size;
			res.headers["Content-Length"] = lenStream.str();
			if (isHead)
				res.body.clear();

			client.setResponse(res.serialize());
			client.setState(Client::WRITING);
			return true;
		} else {
			std::cout << "[STATIC] Error 403: no read permission -> " << filePath << std::endl;
			std::string customErrorPage = "";
			const std::map<int, std::string>& errorPages = loc.getErrorPages();
			if (errorPages.find(403) != errorPages.end()) {
				customErrorPage = resolveCustomErrorPagePath(loc, errorPages.find(403)->second);
			}
			client.setResponse(ErrorBuilder::build(403, customErrorPage));
			client.setState(Client::WRITING);
			return true;
		}
	}
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
		html << "Error: could not open directory.\n";
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
