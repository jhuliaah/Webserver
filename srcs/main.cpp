/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 15:32:55 by ratanaka          #+#    #+#             */
/*   Updated: 2026/08/13 19:15:48 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Config.hpp"
#include "../includes/Server.hpp"
#include "../includes/ConfigParser.hpp"

# include <iostream>

int main(int argc, char* argv[]) {

	/* mudando a condicao aqui. agora podemos
	aceitar com ou sem arquivo de configuracao.
	agora só recusamos se vem mais de um arg. */

	std::string configFile = "config_files/default/config.conf";
	if (argc == 2) { configFile = argv[1]; }
	else if (argc > 2) {
		std::cout << "EX{./webserv <config_file>}" << std::endl;
		return 1;
	}

    try {

		Config config(configFile); 
        Server server(config);
		server.initServer(); // Configura sockaddr_in, faz o bind() e o listen()
		server.serverLoop(); // Inicia o vigia do poll() no loop infinito
		} catch (const std::exception& e) {
			
			/* Captura ConfigException e exibe
			 a mensagem em inglês com o strerror(errno) */
			std::cerr << "FATAL ERROR: " << e.what() << std::endl;
			
			/*Termina com código de
			erro de forma limpa */
			return 1; // 
		}

	return 0;
}
