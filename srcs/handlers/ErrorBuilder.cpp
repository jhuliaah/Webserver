
#include "../../includes/ErrorBuilder.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

ErrorBuilder::ErrorBuilder(){}

ErrorBuilder::~ErrorBuilder(){}

std::string ErrorBuilder::getStatusMessage(int code){
	switch (code) {
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		case 504: return "Gateway Timeout";
		default:  return "Error";
	}
}

std::string ErrorBuilder::getDefaultPage(int code, const std::string& message){
	std::stringstream html;
	html << "<html>\n<head><title>" << code << " " << message << "</title></head>\n";
	html << "<body style=\"text-align: center; margin-top: 50px;\">\n";
	html << "<h1>" << code << " " << message << "</h1>\n";
	html << "<hr>\n<p>Webserv_42 / C++</p>\n";
	html << "</body>\n</html>\n";
	return html.str();
}

std::string ErrorBuilder::resolvePagePath(const std::string& root, const std::string& pagePath)
{
	if (pagePath.empty())
		return "";
	if (pagePath[0] == '/' || pagePath.compare(0, 2, "./") == 0)
		return pagePath;

	std::string r = root.empty() ? "./www" : root;
	if (!r.empty() && r[r.size() - 1] == '/')
		r.erase(r.size() - 1);
	return r + "/" + pagePath;
}

std::string ErrorBuilder::build(int errorCode, const std::string& customPagePath){
	std::string message = getStatusMessage(errorCode);
	std::string body;

	if (!customPagePath.empty()) {
		std::ifstream file(customPagePath.c_str(), std::ios::in | std::ios::binary);
		if (file.is_open()) {
			std::stringstream buffer;
			buffer << file.rdbuf();
			body = buffer.str();
			file.close();
		} else {
			std::cout << "[ERROR BUILDER] Warning: custom error page not found: " << customPagePath << std::endl;
			body = getDefaultPage(errorCode, message);
		}
	} else {
		body = getDefaultPage(errorCode, message);
	}
	std::stringstream response;
	response << "HTTP/1.1 " << errorCode << " " << message << "\r\n";
	response << "Content-Type: text/html; charset=UTF-8\r\n";
	response << "Content-Length: " << body.length() << "\r\n";
	response << "Connection: close\r\n\r\n";
	response << body;

	return response.str();
}
