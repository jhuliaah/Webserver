// Nao to pensando no response por enquanto.

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
		// aqui ainda vai um serializer que eu nao entendi ainda.

	std::string serialize() const;

	private:
		static std::string getStatusMessage(int code);

};

#endif // HTTPRESPONSE_HPP