#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "LocationConfig.hpp"
#include <map>
#include <string>

class ServerConfig {
	public:
		ServerConfig();
		~ServerConfig();

		/*	ainda não parei para pensar o que é public e o que é private
			aqui por isso estou deixando tudo public por enquanto...*/
		int							_port;
		std::string					_host;
		std::string					_name;
		std::string					_root;
		std::string					_index;
		size_t						_max_body_size;
		std::map<int, std::string>	_error_pages;
		std::vector<LocationConfig> _locations;
};

#endif