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

/* esse makeconfig serve para retornar um arquivo de configuração
de acordo com a presença ou não de um argumento na linha de comando.
tem que ficar como função auxiliar, não deve ser um método. */
Config makeConfig(int argc, char* argv[]);

#endif