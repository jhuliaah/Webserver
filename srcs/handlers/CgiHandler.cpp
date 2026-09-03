
#include "../../includes/CgiHandler.hpp"
#include "../../includes/ErrorBuilder.hpp"
#include "../../includes/HttpResponse.hpp"
#include "../../includes/Client.hpp"

#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

static std::string resolve500Page(const LocationConfig& loc)
{
	const std::map<int, std::string>& errorPages = loc.getErrorPages();
	std::map<int, std::string>::const_iterator it = errorPages.find(500);
	if (it == errorPages.end())
		return "";
	return ErrorBuilder::resolvePagePath(loc.getRoot(), it->second);
}

CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

const std::string& CgiHandler::getMethod() const
{
	return _method;
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

void CgiHandler::parseCgiOutput(const std::string& buffer, Client& client)
{
	if (buffer.empty())
	{
		client.setResponse(ErrorBuilder::build(500, client.getCgiContext().errorPage500));
		client.setState(Client::WRITING);
		return;
	}

	if (buffer.compare(0, 5, "HTTP/") == 0)
	{
		std::string response = buffer;
		client.setResponse(response);
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
			continue;

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);
		size_t valueStart = value.find_first_not_of(' ');
		value = (valueStart == std::string::npos) ? "" : value.substr(valueStart);

		if (key == "Status")
			res.status_code = std::atoi(value.c_str());
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

	std::string interpreter = loc.getCgiPath().empty() ? "/usr/bin/python3" : loc.getCgiPath();

	std::string root = loc.getRoot();
	if (root.empty())
		root = "./www";
	if (root[root.size() - 1] == '/')
		root.erase(root.size() - 1);
	std::string scriptName = root + req.getUri();
	std::string errorPage500 = resolve500Page(loc);
	int pipe_in[2];
	int pipe_out[2];

	if (pipe(pipe_in) == -1) {
		std::cerr << "Error creating CGI input pipe" << std::endl;
		client.setResponse(ErrorBuilder::build(500, errorPage500));
		client.setState(Client::WRITING);
		return true;
	}
	if (pipe(pipe_out) == -1) {
		close(pipe_in[0]);
		close(pipe_in[1]);
		std::cerr << "Error creating CGI output pipe" << std::endl;
		client.setResponse(ErrorBuilder::build(500, errorPage500));
		client.setState(Client::WRITING);
		return true;
	}
	pid_t pid = fork();

	if (pid == -1) {
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		std::cerr << "Error forking CGI process" << std::endl;
		client.setResponse(ErrorBuilder::build(500, errorPage500));
		client.setState(Client::WRITING);
		return true;
	}

	if (pid == 0) {
		close(pipe_in[1]);
		close(pipe_out[0]);
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_out[1]);

		size_t slash = scriptName.find_last_of('/');
		std::string scriptDirectory = (slash == std::string::npos) ? "." : scriptName.substr(0, slash);
		std::string scriptFile = (slash == std::string::npos) ? scriptName : scriptName.substr(slash + 1);
		if (chdir(scriptDirectory.c_str()) == -1)
			std::exit(1);
		char* scriptPath = const_cast<char*>(interpreter.c_str());
		char* scriptNameArg = const_cast<char*>(scriptFile.c_str());
		char* argv[] = { scriptPath, scriptNameArg, NULL};
		char** envp = CgiEnvBuilder(req);
		execve(argv[0], argv, envp);
		std::cerr << "CGI execve failed" << std::endl;
		std::exit(1);
	}

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
		cgi.errorPage500 = errorPage500;

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
