/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ratanaka <ratanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/23 12:16:26 by ratanaka          #+#    #+#             */
/*   Updated: 2026/09/03 14:25:50 by ratanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <cstddef>
#include <ctime>
#include <fstream>
#include <sys/types.h>
#include "HttpRequest.hpp"


class Client {
	public:
		enum State {READING, WRITING, CGI_RUNNING, CLOSED};

		struct CgiContext {
			pid_t		pid;
			int			stdin_fd;
			int			stdout_fd;
			std::string outputBuffer;
			size_t		inputSent;
			time_t		startTime;
			std::string	cgiOutput;
			// caminho já resolvido da página de erro 500 custom (config +
			// root da location), calculado uma vez quando o CGI é disparado
			// (CgiHandler::handle, onde a location ainda está à mão) --
			// handleCgiWrite/handleCgiOutput/parseCgiOutput só leem esse
			// campo depois, quando só têm o Client, não a location.
			std::string	errorPage500;

			CgiContext() : pid(-1), stdin_fd(-1), stdout_fd(-1), inputSent(false), startTime(0) {}
		};

	private:
		int				_fd;
		int				_serverFd;
		time_t			_lastActivity;
		std::string		_rawRequest;
		std::string		_response;
		State			_state_e;

		HttpRequest		_request;
		CgiContext		_cgiContext;
		long			_contentLength;

		std::ifstream	_fileStream;
		size_t			_fileSize;
		size_t			_fileBytesSent;
		bool			_isStreamingFile;
		// setResponse() lê o header "Connection" da resposta que está sendo
		// mandada e guarda aqui se ela diz "close". Sem isso o server sempre
		// reciclava a conexão pra READING depois de qualquer resposta
		// completa, mesmo quando a própria resposta (ErrorBuilder, redirect,
		// 504) dizia "Connection: close" -- a conexão nunca fechava de
		// verdade, só o header mentia.
		bool			_shouldClose;
		// client_max_body_size do server{} que aceitou essa conexão (setado
		// por Server::handleNewConnection logo depois do accept). Sentinela
		// "sem limite" enquanto não é setado, pra Client continuar utilizável
		// sozinho. Antes disso não existia -- readData() bufferizava o body
		// inteiro em _rawRequest sem nunca olhar o limite, e só depois de
		// tudo já estar em memória é que dispatchRequest() comparava com
		// getMaxBodySize() e mandava 413. Um client_max_body_size pequeno não
		// impedia um body gigante de ser bufferizado inteiro primeiro.
		size_t			_maxBodySize;
		bool			_bodyTooLarge;
		// decidido por Server::dispatchRequest logo depois do parse (versão
		// HTTP + header Connection da REQUEST), antes de qualquer handler
		// montar a resposta. StaticHandler manda "Connection: keep-alive" em
		// toda resposta 200 sem olhar pra isso -- sem essa flag, o servidor
		// honrava o "Connection: close" da resposta (fix anterior) mas nunca
		// respeitava o que o CLIENT pediu: um GET HTTP/1.0 sem "Connection:
		// keep-alive" explícito devia fechar por padrão, e não fechava.
		// setResponse() usa isso pra decidir _shouldClose de verdade.
		bool			_keepAliveEligible;

	public:
		Client(int fd);
		~Client();

		bool readData();
		bool writeData();
		void prepareNextRequest();
		bool isTimeout(time_t currentTime, int timeoutLimit);
		bool shouldClose() const { return _shouldClose; }
		void setMaxBodySize(size_t size) { _maxBodySize = size; }
		bool isBodyTooLarge() const { return _bodyTooLarge; }
		void setKeepAliveEligible(bool v) { _keepAliveEligible = v; }

		int		getFd() const { return _fd; }
		State	getState() const { return _state_e; }
		void	setState(State state) {_state_e = state;}
		int  getServerFd() const { return _serverFd; }
    	void setServerFd(int fd) { _serverFd = fd; }

		CgiContext& getCgiContext() {return _cgiContext;}
		HttpRequest& getRequest() {return _request;}
		std::string&	getRawRequest() {return _rawRequest;}

		void	setResponse(const std::string& res);

		bool startFileStream(const std::string& filePath, size_t fileSize);
		bool isStreamingFile() const;
		bool sendNextChunk();
		void closeFileStream();
};

#endif



