#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <map>

class HttpRequest
{
	private:
		std::string _method;
		std::string _uri;
		std::string _path;
		std::string _queryString;
		std::string _body;
		std::map<std::string, std::string> _headers;

	public:

		HttpRequest();
		~HttpRequest();

		const std::string& getMethod() const;
		const std::string& getUri() const;
		const std::string& getPath() const;
		const std::string& getQueryString() const;
		const std::string& getBody() const;

		std::string getHeader(const std::string& name) const;
		const std::map<std::string, std::string>& getHeaders() const;

		void setMethod(const std::string& method);
		void setUri(const std::string& uri);
		void setPath(const std::string& path);
		void setQueryString(const std::string& queryString);
		void setBody(const std::string& body);
		void addHeader(const std::string& key, const std::string& value);
};

#endif
