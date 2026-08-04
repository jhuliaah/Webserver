#include "../../includes/Config.hpp"
#include "../../includes/ConfigParser.hpp"
#include "../../includes/Exeptions.hpp"

/* Converte a struct ParsedLocation (mundo do ConfigParser/ConfigTypes.hpp)
para a classe LocationConfig (mundo do Router/Server/StaticHandler). */
static LocationConfig toLocationConfig(const ParsedLocation &src)
{
	LocationConfig loc;

	loc._path = src.path;
	loc._methods = src.methods;
	loc._root = src.root;
	loc._index = src.index;
	loc._autoindex = src.autoindex;
	loc._cgi_extension = src.cgiExt;
	loc._cgi_path = src.cgiPath;
	return loc;
}

/* Converte a struct ParsedServer (ConfigTypes.hpp) para a classe ServerConfig
(ServerConfig.hpp) usada pelo resto do servidor. */
static ServerConfig toServerConfigClass(const ParsedServer &src)
{
	ServerConfig server;

	server._port = src.port;
	server._host = src.host;
	server._name = src.serverName.empty() ? "" : src.serverName[0];
	server._root = src.root;
	server._index = src.index;
	server._max_body_size = src.clientMaxBodySize;
	server._error_pages = src.errorPages;
	for (size_t i = 0; i < src.locations.size(); i++)
		server._locations.push_back(toLocationConfig(src.locations[i]));
	return server;
}

static void loadFromPath(Config &config, const std::string &path)
{
	std::vector<ParsedServer> parsed;

	try {
		parsed = ConfigParser::parse(path);
	} catch (const ConfigParser::ParseException &e) {
		throw ServerException(std::string("config parse failed -> ") + e.what());
	}
	for (size_t i = 0; i < parsed.size(); i++)
		config._servers.push_back(toServerConfigClass(parsed[i]));
}

Config::Config()
{
	loadFromPath(*this, "config_files/default/config.conf");
}

Config::Config(std::string path)
{
	loadFromPath(*this, path);
}

Config::~Config() {}

Config makeConfig(int argc, char *argv[])
{
	if (argc == 2)
		return Config(argv[1]);
	return Config();
}
