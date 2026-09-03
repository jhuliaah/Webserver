#include "../includes/HttpResponse.hpp"
#include <sstream>

HttpResponse::HttpResponse() : status_code(200)
{
}

HttpResponse::~HttpResponse()
{
}

std::string HttpResponse::getStatusMessage(int code)
{
	switch (code)
	{
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		case 504: return "Gateway Timeout";
		default:  return "Unknown";
	}
}

std::string HttpResponse::serialize() const
{
	std::ostringstream out;

	out << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";

	bool hasContentLength = (headers.find("Content-Length") != headers.end());
	bool hasConnection = (headers.find("Connection") != headers.end());

	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		out << it->first << ": " << it->second << "\r\n";
	if (!hasConnection)
		out << "Connection: keep-alive\r\n";

	if (!hasContentLength)
		out << "Content-Length: " << body.length() << "\r\n";

	out << "\r\n" << body;
	return out.str();
}

