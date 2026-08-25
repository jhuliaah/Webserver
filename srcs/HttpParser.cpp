
#include "../includes/HttpParser.hpp"
#include "../includes/HttpResponse.hpp"

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

    size_t sp1 = line.find(' ');
    size_t sp2 = (sp1 != std::string::npos) ? line.find(' ', sp1 + 1) : std::string::npos;
    if (sp1 == std::string::npos || sp2 == std::string::npos)
        return (false);

    std::string method = line.substr(0, sp1);
    std::string target = line.substr(sp1 + 1, sp2 - sp1 - 1);
    std::string version = line.substr(sp2 + 1);

    if (methos.empty() || target.empty() || version.find("HTTP/") !=0)
        return (false);

    size_t qPos = target.fins('?');
    if (qPos != std::string::npos)
    {
        req.setUri(target.substr(0, qPos));
        req.setQueryString(target.substr(qPos + 1));
    }
    else
    {
        req.setUri(target);
        req.setQueryString("");
    }
    req.setMethod(method);
    req.setPath(req.grtUri());
    return (true);
}

bool HttpParser::parseHeadersFrom(const std::string& raw, size_t& pos, HttpRequest& req)
{
	while (true)
	{
		size_t lineEnd = raw.find("\r\n", pos);
		if (lineEnd == std::string::npos)
			return false;

		if (lineEnd == pos) // linha vazia -> fim dos headers
		{
			pos = lineEnd + 2;
			return true;
		}

		std::string line = raw.substr(pos, lineEnd - pos);
		pos = lineEnd + 2;

		size_t colon = line.find(':');
		if (colon == std::string::npos)
			return false; // header sem ":" -> malformado

		std::string key = trim(line.substr(0, colon));
		std::string value = trim(line.substr(colon + 1));
		if (key.empty())
			return false;

		req.addHeader(key, value);
	}
}

void HttpParser::parseBodyFrom(const std::string& raw, size_t pos, HttpRequest& req)
{
	if (pos < raw.size())
		req.setBody(raw.substr(pos));
	else
		req.setBody("");
}

HttpParser::State HttpParser::parse(const std::string& raw, HttpRequest& req)
{
	size_t pos = 0;

	_state = REQUEST_LINE;
	if (!parseRequestLineFrom(raw, pos, req))
	{
		_state = ERROR;
		return _state;
	}

	_state = HEADERS;
	if (!parseHeadersFrom(raw, pos, req))
	{
		_state = ERROR;
		return _state;
	}

	_state = BODY;
	parseBodyFrom(raw, pos, req);

	_state = COMPLETE;
	return _state;
}
