// Essa classe vai funcionar cmo a interface para lidar com request.

#ifndef IREQUESTHANDLER_HPP
#define IREQUESTHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class IRequestHandler
{
	public:
	    virtual ~IRequestHandler(){};
		virtual HttpResponse handle(const HttpRequest& request) = 0;
};

#endif