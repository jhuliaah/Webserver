
#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <map>
#include <string>

class HttpResponse
{
	public:
		HttpResponse();
		~HttpResponse();

		int									status_code;
		std::map<std::string, std::string>	headers;
		std::string							body;

	std::string serialize() const;

	private:
		static std::string getStatusMessage(int code);

};

#endif