
#ifndef IREQUESTHANDLER_HPP
#define IREQUESTHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "LocationConfig.hpp"

class Client;

class IRequestHandler
{
	public:
		virtual ~IRequestHandler() {}

			virtual bool handle(const HttpRequest& req,
							const LocationConfig& loc,
							Client& client) = 0;

};

#endif
