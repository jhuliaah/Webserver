
#ifndef CONFIG_TYPER_HPP
#define CONFIG_TYPER_HPP

#include <string>
#include <vector>
#include <map>

typedef std::string string;

struct location
{
    string  path;
    string  root;
    string  index;
    string  redirect;
    string  uploadDir;
    std::vector<string> methods;
    bool    autoindex;
    std::map<string, string> cgiExtensions;

    Location()
        : autoindex(false), uploadEnable(false) {}
};

struct ServerConfig
{
    string                  host;
    int                     port;
    std::vector<string>     serverName;
    std::map<int, string>   errorPages;
    size_t                  clientMaxBodySize;
    std::vector<Location>   locations;

    serverConfig()
        : port(0), clientMaxBodySize(1048576) {}
};

endif;