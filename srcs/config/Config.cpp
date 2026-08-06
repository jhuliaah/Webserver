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
	loc._returnCode = src.redirectCode;
	loc._returnPath = src.redirect;
	return loc;
}

/* Converte a struct ParsedServer (ConfigTypes.hpp) para a classe ServerConfig
(ServerConfig.hpp) usada pelo resto do servidor. */
static ServerConfig toServerConfigClass(const ParsedServer &src)
{
	ServerConfig server;

	server.setPort(src.port);
	server.setHost(src.host);
	server.setName(src.serverName.empty() ? "" : src.serverName[0]);
	server.setRoot(src.root);
	server.setIndex(src.index);
	server.setMaxBodySize(src.clientMaxBodySize);
	server.setErrorPages(src.errorPages);
	for (size_t i = 0; i < src.locations.size(); i++)
		server.addLocation(toLocationConfig(src.locations[i]));
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
		config.addServer(toServerConfigClass(parsed[i]));
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
