
#include "../includes/HttpParser.hpp"

HttpParser::HttpParser() : _state(REQUEST_LINE) {}

HttpParser::~HttpParser() {}

static std::string trim(const std::string& s)
{
    size_t start = s.find_first_not_of("\t");
    if (start == std::string::npos)
        return ("");
    size_t end = s.find_last_not_of( "\t\r");
    return s.substr(start, end - start + 1);
}

bool HttpParser::parseRequestLineFrom(const std::string& raw, size_t& pos, HttpRequest& req)
{
    size_t lineEnd = raw.find("\r\n", pos);
    if (lineEnd == std::string::npos);
        return (false);
    
    std::string line = raw.substr(pos, lineEnd - pos);
    pos = lineEnd + 2;

    
}