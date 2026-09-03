#include "../../includes/Config.hpp"
#include "../../includes/ConfigParser.hpp"
#include "../../includes/Exeptions.hpp"

static LocationConfig toLocationConfig(const ParsedLocation &src, const ParsedServer &parent)
{
	LocationConfig loc;

	loc.setPath(src.path);
	loc.setMethods(src.methods);
	loc.setRoot(src.root.empty() ? parent.root : src.root);
	loc.setIndex(src.index.empty() ? parent.index : src.index);
	loc.setAutoindex(src.autoindex);
	loc.setCgiExtension(src.cgiExt);
	loc.setCgiPath(src.cgiPath);
	loc.setReturnCode(src.redirectCode);
	loc.setReturnPath(src.redirect);
	loc.setUploadDir(src.uploadDir);
	loc.setErrorPages(parent.errorPages);
	return loc;
}

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
		server.addLocation(toLocationConfig(src.locations[i], src));
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
