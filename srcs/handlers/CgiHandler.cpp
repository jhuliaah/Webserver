/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 17:45:48 by eduribei          #+#    #+#             */
/*   Updated: 2026/08/13 21:30:46 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/CgiHandler.hpp"
#include "../../includes/ErrorBuilder.hpp"
#include "../../includes/HttpResponse.hpp"
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

/*
	Monta a resposta HTTP a partir do que o script CGI escreveu no stdout.

	Dois formatos aceitos:
	1) O script já manda a status line HTTP inteira por conta própria
	   (ex.: os scripts de teste em cgi-bin/, que começam com
	   "HTTP/1.1 200 OK\r\n..."). Nesse caso repassa direto -- prefixar
	   outra status line por cima duplicava a linha e quebrava o response.
	2) Formato CGI/1.1 "de verdade": só headers ("Chave: valor", um por
	   linha), linha em branco, body -- sem status line. Se o script
	   mandar um header "Status: 404 Not Found", esse vira o status code
	   da resposta; sem "Status:", o padrão é 200. Aceita tanto "\r\n\r\n"
	   quanto só "\n\n" como separador de headers/body, porque scripts
	   simples com print() só mandam "\n".
*/
void CgiHandler::parseCgiOutput(const std::string& buffer, Client& client)
{
	if (buffer.empty())
	{
		client.setResponse(ErrorBuilder::build(500, ""));
		client.setState(Client::WRITING);
		return;
	}

	if (buffer.compare(0, 5, "HTTP/") == 0)
	{
		client.setResponse(buffer);
		client.setState(Client::WRITING);
		return;
	}

	size_t sepPos = buffer.find("\r\n\r\n");
	size_t sepLen = 4;
	if (sepPos == std::string::npos)
	{
		sepPos = buffer.find("\n\n");
		sepLen = 2;
	}

	HttpResponse res;
	res.status_code = 200;

	if (sepPos == std::string::npos)
	{
		// Não achou fim de headers -> trata tudo como body cru.
		res.headers["Content-Type"] = "text/html";
		res.body = buffer;
		client.setResponse(res.serialize());
		client.setState(Client::WRITING);
		return;
	}

	std::string headerBlock = buffer.substr(0, sepPos);
	std::string body = buffer.substr(sepPos + sepLen);
	std::string lineSep = (headerBlock.find("\r\n") != std::string::npos) ? "\r\n" : "\n";

	size_t pos = 0;
	while (pos < headerBlock.size())
	{
		size_t lineEnd = headerBlock.find(lineSep, pos);
		std::string line = (lineEnd == std::string::npos)
			? headerBlock.substr(pos)
			: headerBlock.substr(pos, lineEnd - pos);
		pos = (lineEnd == std::string::npos) ? headerBlock.size() : lineEnd + lineSep.size();

		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue; // linha sem ":" -> ignora, não é header válido

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);
		size_t valueStart = value.find_first_not_of(' ');
		value = (valueStart == std::string::npos) ? "" : value.substr(valueStart);

		if (key == "Status")
			res.status_code = std::atoi(value.c_str()); // "404 Not Found" -> 404
		else
			res.headers[key] = value;
	}

	res.body = body;
	client.setResponse(res.serialize());
	client.setState(Client::WRITING);
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
		fcntl(pipe_out[0], F_SETFD, FD_CLOEXEC);

		Client::CgiContext& cgi = client.getCgiContext();
		cgi.pid = pid;
		cgi.stdout_fd = pipe_out[0];
		cgi.startTime = time(NULL);
		cgi.inputSent = 0;

		if (req.getMethod() == "POST") {
			fcntl(pipe_in[1], F_SETFL, O_NONBLOCK);
			fcntl(pipe_in[1], F_SETFD, FD_CLOEXEC);
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
