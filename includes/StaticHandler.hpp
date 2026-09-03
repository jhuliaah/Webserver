
#ifndef STATIC_HANDLER_HPP
# define STATIC_HANDLER_HPP

# include <string>
# include "IRequestHandler.hpp"
# include "HttpRequest.hpp"
# include "Client.hpp"

class StaticHandler {
	private:
		std::string getMimeType(const std::string& path);
		std::string buildAutoIndex(const std::string& path, const std::string& uri);

	public:
		StaticHandler();
		~StaticHandler();

		bool handle(const HttpRequest& req, const LocationConfig& loc, Client& client);
};

#endif