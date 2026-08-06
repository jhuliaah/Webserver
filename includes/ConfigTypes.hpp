
#ifndef CONFIG_TYPER_HPP
#define CONFIG_TYPER_HPP

#include <string>
#include <vector>
#include <map>

typedef std::string string;

struct ParsedLocation
{
    string  path;
    string  root;
    string  index;
    string  redirect;
    string  uploadDir;
    string  cgiPath;
    string  cgiExt;
    std::vector<string> methods;
    int     redirectCode;
    bool    autoindex;

    ParsedLocation()
        : redirectCode(0), autoindex(false) {}
};

struct ParsedServer
{
    string                  host;
    int                     port;
    string                  root;
    string                  index;
    std::vector<string>     serverName;
    std::map<int, string>   errorPages;
    size_t                  clientMaxBodySize;
    std::vector<ParsedLocation>   locations;

    ParsedServer()
        : port(0), clientMaxBodySize(1048576) {}
};

#endif
