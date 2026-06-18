/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 15:32:55 by ratanaka          #+#    #+#             */
/*   Updated: 2026/05/28 14:40:46 by ratanaka         ###   ########.fr       */
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

		server.initServer(); // Configura a sockaddr_in, faz o bind() e o listen()
		server.serverLoop(); // Inicia o vigia do poll() no loop infinito

	} catch (const std::exception& e) {
		// Captura SocketException ou ServerException e exibe a mensagem em inglês com o strerror(errno)
		std::cerr << "FATAL ERROR: " << e.what() << std::endl;
		return 1; // Termina com código de erro de forma limpa
	}
	return 0;
}