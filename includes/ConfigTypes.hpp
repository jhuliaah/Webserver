
#ifndef CONFIG_TYPER_HPP
#define CONFIG_TYPER_HPP

#include <string>
#include <vector>
#include <map>

typedef std::string string;

/* Renomeados de `location`/`ServerConfig` para `ParsedLocation`/
`ParsedServer`: esses nomes colidiam com as classes `LocationConfig`/
`ServerConfig` (LocationConfig.hpp/ServerConfig.hpp) já usadas pelo Router,
Server e Config — dois tipos com o mesmo nome no escopo global não compilam
quando os dois mundos precisam ser incluídos juntos (ex: Config.cpp). */
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
    bool    autoindex;

    ParsedLocation()
        : autoindex(false) {}
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
