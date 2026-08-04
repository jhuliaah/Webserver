// Aqui só um esqueleto, mas tem mais coisa no webserv do Fabio.

#include "../includes/HttpRequest.hpp"

HttpRequest::HttpRequest() {}

HttpRequest::~HttpRequest() {}

const std::string& HttpRequest::getMethod() const { return _method; }
const std::string& HttpRequest::getUri() const { return _uri; }
const std::string& HttpRequest::getPath() const { return _path; }
const std::string& HttpRequest::getQueryString() const { return _queryString; }
const std::string& HttpRequest::getBody() const { return _body; }
const std::map<std::string, std::string>& HttpRequest::getHeaders() const { return _headers; }

std::string HttpRequest::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(name);
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

void HttpRequest::setMethod(const std::string& method) { _method = method; }
void HttpRequest::setUri(const std::string& uri) { _uri = uri; }
void HttpRequest::setPath(const std::string& path) { _path = path; }
void HttpRequest::setQueryString(const std::string& queryString) { _queryString = queryString; }
void HttpRequest::setBody(const std::string& body) { _body = body; }

void HttpRequest::addHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}
