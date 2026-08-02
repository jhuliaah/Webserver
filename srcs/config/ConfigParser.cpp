
#include "../../includes/ConfigParser.hpp"
#include "../../includes/Tokenizer.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>

//Perror
void ConfigParser::expect(const std::vector<string> &tokens, size_t i, const string &value)
{
    if (i >= tokens.size() || tokens[i] !=value)
        throw ParseException(value + "expected in config");
}

//puts in a vector every word of the config file
std::vector<ServerConfig> ConfigParser::parse(const string &path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
        throw ParseException("it wasn't possible to open the config file " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::vector<string> tokens = Tokenizer::tokenize(buffer.str());
    return parseTokens(tokens);
}

std::vector<ServerConfig> ConfigParser::parseTokens(const std::vector<string> &tokens)
{
    std::vector<ServerConfig> servers;
    size_t i = 0;

    while (i <tokens.size())
    {
        if (tokens[i] == "server")
        {
            i++;
            expect(tokens, i, "{");
            i++;
            servers.push_back(parseServerBlock(tokens, i));
            expect(tokens, i, "}");
            i++;
        }
        else
            throw ParseException("unexpected token outsied of the server block: " + tokens[i]);
    }

    if (servers.empty())
        throw ParseException("no server block found in the config file");
    
    return (servers);
} 

ServerConfig ConfigParser::parseServerBlock(const std::vector<string> &tokens, size_t &i)
{
    ServerConfig server;

    while (i <tokens.size() && tokens[i] != "}")
    {
        const string &key = tokens[i];

        if (key == "listen")
        {
            i++;
            string value = tokens[i];
            size_t separator = value.find(':');
            if (separator != std::string::npos)
            {
                server.host = value.substr(0, separator);
                server.port = std::atoi(value.substr(separator + 1).c_str());
            }
            else
            {
                server.host = "0.0.0.0";
                server.port = std::atoi(value.substr(separator + 1).c_str());
            }
            i++;
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "server_name")
        {
            i++;
            while (i < tokens.size() && tokens[i] != ";")
            {
                server.serverName.push_back(tokens[i]);
                i++;
            }
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "error_page")
        {
            i++;
            std::vector<string> codes;
            while (i < tokens.size() && tokens[i] != ";")
            {
                codes.push_back(tokens[i]);
                i++;
            }
            expect(tokens, i, ";");

            //verifying the code and a path error
            if (codes.size() >= 2)
            {
                const string &pagePath = codes.back();
                for (size_t c = 0; c + 1 < codes.size(); ++c)
                    server.errorPages[std::atoi(codes[c].c_str())] = pagePath; 
            }
        }
        else if (key == "client_max_body_size")
        {
            i++;
            //casting the string to size_t 
            server.clientMaxBodySize = static_cast<size_t>(std::atol(tokens[i].c_str()));
            i++;
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "location")
        {
            i++;
            string path = tokens[i];
            i++;
            expect(tokens, i, "{");
            i++;
            location loc = parseLocationBlock(tokens, i);
            loc.path = tokens[i];
            expect(tokens, i, "}");
            i++;
            server.locations.push_back(loc);
        }
        else
            throw ParseException("Unkown token in the server block: " + key);
    }
    return (server);
}

location ConfigParser::parseLocationBlock(const std::vector<string> &tokens, size_t &i)
{
    location loc;

    while (i < tokens.size() && tokens[i] != "}")
    {
        const string &key = tokens[i];

        if (key == "methods")
        {
            i++;
            while (i < tokens.size() && tokens[i] != ";")
            {
                loc.methods.push_back(tokens[i]);
                i++;
            }
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "root")
        {
            i++;
            loc.root = tokens[i];
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "index")
        {
            i++;
            loc.index == tokens[i];
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "autoindex")
        {
            i++;
            loc.autoindex = (tokens[i] == "on");
            i++;
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "return")
        {
            i++;
            loc.redirect = tokens[i];
            i++;
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "upload_dir")
        {
            i++;
            loc.uploadDir = tokens[i];
            i++;
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "cgi_extension")
        {
            i++;
            string ext = tokens[i];
            i++;
            string interpreter = tokens[i];
            i++;
            loc.cgiExtensions[ext] = interpreter;
            expect(tokens, i, ";");
            i++;
        }
        else
            throw ParseException("unknown token in Location block: " + key);
    }
    return (loc);
}