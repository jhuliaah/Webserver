
#ifndef DELETEHANDLER_HPP
# define DELETEHANDLER_HPP

# include "HttpRequest.hpp"
# include "Client.hpp"
# include "LocationConfig.hpp"
# include <string>
#include "UploadHandler.hpp"

class DeleteHandler {
	public:
		DeleteHandler();
		~DeleteHandler();

		bool handle(const HttpRequest& req, const LocationConfig& loc, Client& client);
};

#endif