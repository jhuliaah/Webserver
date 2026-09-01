#include "../../includes/DeleteHandler.hpp"
#include "../../includes/HttpResponse.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

DeleteHandler::DeleteHandler() {}
DeleteHandler::~DeleteHandler() {}

bool DeleteHandler::handle(const HttpRequest& req, const LocationConfig& loc, Client& client){
	std::string root = loc.getRoot().empty() ? "./www" : loc.getRoot();
	std::string uri = req.getUri();
	if (!root.empty() && root[root.size() - 1] == '/')
		root.erase(root.size() - 1);
	if (uri.empty())
		uri = "/";
	std::string filePath = root + (uri[0] == '/' ? uri : std::string("/") + uri);

	struct stat pathStat;
	HttpResponse res;
	res.headers["Content-Type"] = "text/html";

	if (stat(filePath.c_str(), &pathStat) == -1) {
		std::cout << "[DELETE] File not found: " << filePath << std::endl;
		res.status_code = 404;
		res.body = "<html><body><center><h1>404 Not Found</h1></center></body></html>";
	}
	else if (S_ISDIR(pathStat.st_mode)) {
		std::cout << "[DELETE] Cannot delete directory: " << filePath << std::endl;
		res.status_code = 403;
		res.body = "<html><body><center><h1>403 Forbidden</h1></center></body></html>";
	}
	else if (access(filePath.c_str(), W_OK) == -1) {
		std::cout << "[DELETE] No permission to delete: " << filePath << std::endl;
		res.status_code = 403;
		res.body = "<html><body><center><h1>403 Forbidden</h1></center></body></html>";
	}
	else if (unlink(filePath.c_str()) == 0) {
		std::cout << "[DELETE] File deleted successfully: " << filePath << std::endl;
		res.status_code = 204;
		res.headers.erase("Content-Type");
	}
	else {
		std::cout << "[DELETE] Internal error while deleting file." << std::endl;
		res.status_code = 500;
		res.body = "<html><body><center><h1>500 Internal Server Error</h1></center></body></html>";
	}

	client.setResponse(res.serialize());
	client.setState(Client::WRITING);
	return true;
}
