
#include "../includes/HttpParser.hpp"

HttpParser::HttpParser() : _state(REQUEST_LINE) {}

HttpParser::~HttpParser() {}

bool HttpParser::parseRequestLineFrom(const std::string& raw, size_t& pos, HttpRequest& req)
{
    (void)req;
    size_t lineEnd = raw.find("\r\n", pos);
    if (lineEnd == std::string::npos)
        return (false);

    std::string line = raw.substr(pos, lineEnd - pos);
    pos = lineEnd + 2;

    return (false);
}