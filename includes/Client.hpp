
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

		bool			_shouldClose;

		size_t			_maxBodySize;
		bool			_bodyTooLarge;

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
