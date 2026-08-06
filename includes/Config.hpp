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

		const std::vector<ServerConfig>& getServers() const { return _servers; }
		void addServer(const ServerConfig& server) { _servers.push_back(server); }

	private:
		std::vector<ServerConfig> _servers;
};

Config makeConfig(int argc, char* argv[]);

#endif