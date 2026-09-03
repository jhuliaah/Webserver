
#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

# include <string>
# include "IRequestHandler.hpp"

class CgiHandler : public IRequestHandler
{
	private:
		std::string _method;

	public:
		CgiHandler();
		~CgiHandler();

		const std::string& getMethod() const;

		char	**CgiEnvBuilder(const HttpRequest& req);
		static void parseCgiOutput(const std::string& buffer, Client& client);
		bool	handle(const HttpRequest& req, const LocationConfig& loc, Client& client);

};
#endif