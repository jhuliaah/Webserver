
#ifndef ERRORBUILDER_HPP
# define ERRORBUILDER_HPP

# include <string>

class ErrorBuilder
{
private:
	ErrorBuilder();
	~ErrorBuilder();

	static std::string getStatusMessage(int code);
	static std::string getDefaultPage(int code, const std::string& message);

public:
	static std::string build(int errorCode, const std::string& customPagePath = "");

	static std::string resolvePagePath(const std::string& root, const std::string& pagePath);
};

#endif