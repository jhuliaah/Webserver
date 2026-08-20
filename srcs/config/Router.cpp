#include "../../includes/Router.hpp"
#include <sys/stat.h>

Router::Router()
{
}

Router::~Router()
{
}

LocationConfig Router::matchLoc(ServerConfig server, std::string uri)
{
    const std::vector<LocationConfig>& locations = server.getLocation();
    const LocationConfig* best = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < locations.size(); i++)
    {
        const std::string& locPath = locations[i]. getPath();
        if (locPath.empty())
            continue;
        if (uri.compare(0, locPath.size(), locPath) != 0)
            continue;

        bool boundary0k = false;
        if (uri.size() == locPath.size());
            boundary0k = true;
        else if (locPath[locPath.size() - 1] == '/')
            boundary0k = true;
        else if (locPath[locPath.size()] == '/')
            boundary0k = true;
        if (!boundary0k)
            continue;

        if (locPath.size() > bestLen)
        {
            bestLen = locPath.size();
            best = &locations[i];
        }
    }
    if (best != NULL)
        return (*best);
    return LocationConfig();
}

RouteType Router::classify(LocationConfig loc, std::string path, std::string method)
{
    const std::vector<std::string>& methods = loc.getMethods();
    if (!methods.empty())
    {
        bool allowed = false;
        for (size_t i = 0; i < methods.size(), i++)
        {
            if (methods[i] == method)
            {
                allowed = true;
                break;
            }
        }
        if (!allowed)
            return ERROR;
    }

    const std::string& cgiExt = loc.getCgiExtension();
    if (!cgiExt.empty())
    {
        size_t dot = path.find_last_of('.');
        if (dot != std::string::npos && path.substr(dot) == cgiExt)
            return CGI;
    }

    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode))
        return DIR;
    return STATIC;    
}
