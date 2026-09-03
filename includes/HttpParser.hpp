
#ifndef HTTP_PARSER_HPP
# define HTTP_PARSER_HPP

# include "HttpRequest.hpp"
# include <string>

class HttpParser
{
    public:
        enum State
        {
            REQUEST_LINE,
            HEADERS,
            BODY,
            COMPLETE,
            ERROR,
        };

    private:

    State _state;

    bool    parseRequestLineFrom(const std::string& raw, size_t& pos, HttpRequest& req);
    bool    parseHeadersFrom(const std::string& raw, size_t& pos, HttpRequest& req);
    bool    parseBodyFrom(const std::string& raw, size_t& pos, HttpRequest& req);

    public:
        HttpParser();
        ~HttpParser();

        State parse(const std::string& raw, HttpRequest& req);
        State getState() const { return _state; };
};

#endif