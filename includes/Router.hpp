#ifndef ROUTER_HPP
# define ROUTER_HPP

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

enum RouteType
{
	STATIC,
	CGI,
	DIR,
	ERROR
};

class Router
{
	private:
		Router();
		~Router();

	public:
		static LocationConfig matchLoc(ServerConfig server, std::string uri);

		static RouteType classify(LocationConfig loc, std::string path, std::string method);

};

#endif
