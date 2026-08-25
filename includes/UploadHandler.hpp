
#ifndef UPLOADHANDLER_HPP
# define UPLOADHANDLER_HPP

# include "HttpRequest.hpp"
# include "Client.hpp"
# include "LocationConfig.hpp"
# include <string>

class UploadHandler {
	public:
		UploadHandler();
		~UploadHandler();

		bool handle(const HttpRequest& req, const LocationConfig& loc, Client& client);
};

#endif