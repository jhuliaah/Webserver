#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <map>

class HttpRequest
{
	public: // Por enquanto está public para me facilitar nos testes.
		std::string method;
		std::string uri;
		std::string path;
		std::string query_string;
		std::string body;
		std::map<std::string, std::string> headers;

		HttpRequest();
		~HttpRequest();

		std::string get_header(const std::string& name) const;
};

#endif
