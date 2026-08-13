/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 17:45:48 by eduribei          #+#    #+#             */
/*   Updated: 2026/08/13 20:43:18 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/CgiHandler.hpp"
#include "../../includes/Client.hpp"

#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>


CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

const std::string& CgiHandler::getMethod() const
{
    /* TODO */
    return _method; /* TODO */
}

char **CgiHandler::CgiEnvBuilder(const HttpRequest& req)
{
	std::vector<std::string> envList;

	envList.push_back("REQUEST_METHOD=" + req.getMethod());
	envList.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envList.push_back("PATH_INFO=" + req.getUri());
	envList.push_back("QUERY_STRING=" + req.getQueryString());

	std::string contentLength = req.getHeader("Content-Length");
	if (!contentLength.empty()) {
		envList.push_back("CONTENT_LENGTH=" + contentLength);
	}
	
	std::string contentType = req.getHeader("Content-Type");
	if (!contentType.empty()) {
		envList.push_back("CONTENT_TYPE=" + contentType);
	}
	
	char** envp = new char*[envList.size() + 1];
	for (size_t i = 0; i < envList.size(); ++i){
		envp[i] = new char[envList[i].size() + 1];
		std::strcpy(envp[i], envList[i].c_str());
	}
	envp[envList.size()] = NULL;
	return envp;
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
	// use location config values (if present) to determine CGI interpreter and script
	std::string interpreter = loc.getCgiPath().empty() ? "/usr/bin/python3" : loc.getCgiPath();
	std::string scriptName = req.getUri();
	if (!scriptName.empty() && scriptName[0] == '/')
		scriptName = std::string(".") + scriptName; // make relative path like ./cgi-bin/script.py
	int pipe_in[2]; // Servidor escreve [1] -> python le [0]
	int pipe_out[2]; // python escreve [1] -> servidor le [0]

	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
		std::cerr << "Error creating CGI pipes" << std::endl;
		return true; // TODO: implement 500 error later
	}
	pid_t pid = fork();

	if (pid == -1) {
		std::cerr << "Error forking CGI process" << std::endl;
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

		char* scriptPath = const_cast<char*>(interpreter.c_str());
		char* scriptNameArg = const_cast<char*>(scriptName.c_str());
		char* argv[] = { scriptPath, scriptNameArg, NULL};
		char** envp = CgiEnvBuilder(req);
		execve(argv[0], argv, envp);
		std::cerr << "CGI execve failed" << std::endl;
		std::exit(1);
	}
	//processo pai (o servidor epoll)
	else {
		close(pipe_in[0]);
		close(pipe_out[1]);

		fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);

		Client::CgiContext& cgi = client.getCgiContext();
		cgi.pid = pid;
		cgi.stdout_fd = pipe_out[0];
		cgi.startTime = time(NULL);
		cgi.inputSent = 0;
		
		if (req.getMethod() == "POST") {
			fcntl(pipe_in[1], F_SETFL, O_NONBLOCK);
			cgi.stdin_fd = pipe_in[1];
		} else {
			close(pipe_in[1]);
			cgi.stdin_fd = -1;
		}

		client.setState(Client::CGI_RUNNING);
		std::cout << "[CGI] Started! PID: " << pid << " | stdout FD: " << cgi.stdout_fd << std::endl;
		return false;
	}
}
