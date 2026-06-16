/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 15:32:55 by ratanaka          #+#    #+#             */
/*   Updated: 2026/06/16 19:12:17 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/WebServer.hpp"

int main(int argc, char* argv[]){
	if (argc != 2){
		std::cout << "EX{./webserv <config_file>}" << std::endl;
		return 1;
	}
	(void)argv;
	
	try {
		Server server; // Cria a instância (chama o construtor do Socket implicitamente)

		std::vector<int> fake_config_ports;
		fake_config_ports.push_back(8080);
		fake_config_ports.push_back(8085);
		fake_config_ports.push_back(9005);
		
		server.initServer(fake_config_ports); // Configura a sockaddr_in, faz o bind() e o listen()
		server.serverLoop(); // Inicia o vigia do poll() no loop infinito

	} catch (const std::exception& e) {
		// Captura SocketException ou ServerException e exibe a mensagem em inglês com o strerror(errno)
		std::cerr << "FATAL ERROR: " << e.what() << std::endl;
		return 1; // Termina com código de erro de forma limpa
	}
	return 0;
}