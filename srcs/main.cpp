
#include "../includes/Config.hpp"
#include "../includes/Server.hpp"
#include "../includes/ConfigParser.hpp"

# include <iostream>

int main(int argc, char* argv[]) {

	std::string configFile = "config_files/default/config.conf";
	if (argc == 2) { configFile = argv[1]; }
	else if (argc > 2) {
		std::cout << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

    try {

		Config config(configFile);
        Server server(config);
		server.initServer();
		server.serverLoop();
		} catch (const std::exception& e) {

			std::cerr << "FATAL ERROR: " << e.what() << std::endl;

			return 1;
		}

	return 0;
}
