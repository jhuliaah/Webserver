#ifndef ROUTE_CONFIG_HPP
# define ROUTE_CONFIG_HPP

# include <string>

class RouteConfig
{
	public:
		std::string path_prefix;
		std::string root;
		std::string cgi_extension;
		std::string cgi_path;
		bool        is_cgi;

		RouteConfig();
		~RouteConfig();
};

#endif