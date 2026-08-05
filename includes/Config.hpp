#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ServerConfig.hpp"
#include <string>
#include <vector>

class Config {
	public:
		Config();
		Config(std::string);
		~Config();

		std::vector<ServerConfig> _servers;

};

Config makeConfig(int argc, char* argv[]);

#endif