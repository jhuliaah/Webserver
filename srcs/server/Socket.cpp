/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 17:38:28 by ratanaka          #+#    #+#             */
/*   Updated: 2026/05/28 10:15:35 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/WebServer.hpp"

//AF_INET — família de endereços IPv4 (endereços tipo 192.168.0.1)
//SOCK_STREAM — tipo TCP — conexão confiável, com garantia de entrega e ordem. O HTTP precisa disso.
//0 — protocolo automático.

Socket::Socket(){
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1)
		throw SocketException("ServerFd failed -> " + std::string(strerror(errno)));
	SocketConfig();
	nonBlocking();
};

//=========================//
// Configuraçoes do Socket //
//=========================//

/* Quando seu servidor para e você tenta reiniciá-lo logo em seguida, o sistema operacional
	ainda guarda a porta "ocupada" por alguns segundos (estado TIME_WAIT do TCP).
	Sem SO_REUSEADDR, o bind() falharia com "Address already in use".
	Com essa opção, o SO libera a porta imediatamente.*/

// O opt é como se fosse um interruptor / 1 ou 0
	
void	Socket::SocketConfig(){
	int	opt = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw SocketException("Configuration failed -> " + std::string(strerror(errno)));
}

/*Por padrão, operações de rede bloqueiam — ou seja, se você chamar recv()
	e não tiver dados ainda, o programa para e fica esperando.
	Para um servidor que atende múltiplos clientes, isso é catastrófico.*/

/*recv() sem dados disponíveis → retorna -1 imediatamente (errno = EWOULDBLOCK)
									em vez de ficar travado esperando*/
	
void	Socket::nonBlocking(){
	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1)
		throw SocketException("NonBlocking failed -> " + std::string(strerror(errno)));
}