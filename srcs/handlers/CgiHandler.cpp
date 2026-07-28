/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 17:45:48 by eduribei          #+#    #+#             */
/*   Updated: 2026/07/23 16:34:30 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/CgiHandler.hpp"
#include "../../includes/WebServer.hpp"
#include "../../includes/Client.hpp"

#include <cstdlib>


CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

const std::string& CgiHandler::getMethod() const
{
    /* TODO */
    return _method; /* TODO */
}

char **CgiHandler::CgiEnvBuilder()
{
    /* TODO */
    return NULL; /* TODO */
}

void CgiHandler::parseCgiOutput(const std::string& buffer, HttpResponse& response)
{
    (void)buffer;
    (void)response;
    
    /* TODO */
    /*  essa funçao vai tratar a saida do CGI e preencher o objeto HttpResponse 
        que vive dentro do objeto Client, que o giHandler::handle() recebeu. */
}

bool CgiHandler::handle(const HttpRequest& req, const LocationConfig& loc, Client& client)
{
	(void)req;
	(void)loc;
	int pipe_in[2]; // Servidor escreve [1] -> python le [0]
	int pipe_out[2]; // python escreve [1] -> servidor le [0]

	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
		std::cerr << "Erro ao criar pipes do CGI" << std::endl;
		return true; //depois implementamos o erro 500
	}
	pid_t pid = fork();

	if (pid == -1) {
		std::cerr << "Erro no fork do CGI" << std::endl;
		return true;
	}

	//processo filho (vira o python)
	if (pid == 0) {
		close(pipe_in[1]);
		close(pipe_out[0]);
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_out[1]);

		char* scriptPath = (char*)"/usr/bin/python3";
		char* scriptName = (char*)"./cgi-bin/script1.py";
		char* argv[] = { scriptPath, scriptName, NULL};
		extern char **environ;
		execve(argv[0], argv, environ);
		std::cerr << "CGI falhou no execve" << std::endl;
		std::exit(1);
	}
	//processo pai (o servidor epoll)
	else {
		close(pipe_in[0]);
		close(pipe_out[1]);
		close(pipe_in[1]);
		// fcntl(pipe_in[1], F_SETFL, O_NONBLOCK);
		fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);

		Client::CgiContext& cgi = client.getCgiContext();
		cgi.pid = pid;
		cgi.stdin_fd = pipe_in[1];
		cgi.stdout_fd = pipe_out[0];
		cgi.startTime = time(NULL);
		
		client.setState(Client::CGI_RUNNING);
		std::cout << "[CGI] Disparado! PID: " << pid << " | FD Tubo: " << cgi.stdout_fd << std::endl;
		return false;
	}
}
