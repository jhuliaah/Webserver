
#include "../../includes/ConfigParser.hpp"
#include "../../includes/Tokenizer.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <cctype>

//Perror
void ConfigParser::expect(const std::vector<string> &tokens, size_t i, const string &value)
{
    if (i >= tokens.size() || tokens[i] !=value)
        throw ParseException(value + "expected in config");
}

//puts in a vector every word of the config file
std::vector<ParsedServer> ConfigParser::parse(const string &path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
        throw ParseException("it wasn't possible to open the config file " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::vector<string> tokens = Tokenizer::tokenize(buffer.str());
    return parseTokens(tokens);
}

std::vector<ParsedServer> ConfigParser::parseTokens(const std::vector<string> &tokens)
{
    std::vector<ParsedServer> servers;
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

ParsedServer ConfigParser::parseServerBlock(const std::vector<string> &tokens, size_t &i)
{
    ParsedServer server;
    bool rootSeen = false;

    while (i <tokens.size() && tokens[i] != "}")
    {
        const string &key = tokens[i];

        if (key == "root")
        {
            if (rootSeen)
                throw ParseException("duplicate root directive in server block");
            i++;
            server.root = tokens[i];
            rootSeen = true;
            i++;
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "index")
        {
            i++;
            server.index = tokens[i];
            i++;
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "listen")
        {
            i++;
            string value = tokens[i];
            size_t separator = value.find(':');
            if (separator != std::string::npos)
            {
                server.host = value.substr(0, separator);
                string portStr = value.substr(separator + 1);
                if (!isValidIPv4(server.host))
                    throw ParseException("invalid IP in listen: " + server.host);
                if (!isValidPort(portStr))
                    throw ParseException("invalid port in listen: " + portStr);
                server.port = std::atoi(portStr.c_str());
            }
            else
            {
                server.host = "0.0.0.0";
                if (!isValidPort(value))
                    throw ParseException("invalid port in listen: " + value);
                server.port = std::atoi(value.c_str());
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
            i++;

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
            server.clientMaxBodySize = parseSize(tokens[i]);
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
            ParsedLocation loc = parseLocationBlock(tokens, i);
            loc.path = path;
            expect(tokens, i, "}");
            i++;

            for (size_t l = 0; l < server.locations.size(); ++l)
                if (server.locations[l].path == loc.path)
                    throw ParseException("duplicate location path: " + loc.path);

            server.locations.push_back(loc);
        }
        else
            throw ParseException("Unkown token in the server block: " + key);
    }
    return (server);
}

ParsedLocation ConfigParser::parseLocationBlock(const std::vector<string> &tokens, size_t &i)
{
    ParsedLocation loc;

    while (i < tokens.size() && tokens[i] != "}")
    {
        const string &key = tokens[i];

        if (key == "allow_methods")
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
            i++;
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "index")
        {
            i++;
            loc.index = tokens[i];
            i++;
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
            loc.redirectCode = std::atoi(tokens[i].c_str());
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
        else if (key == "cgi_path")
        {
            i++;
            loc.cgiPath = tokens[i];
            i++;
            expect(tokens, i, ";");
            i++;
        }
        else if (key == "cgi_ext")
        {
            i++;
            loc.cgiExt = tokens[i];
            i++;
            expect(tokens, i, ";");
            i++;
        }
        else
            throw ParseException("unknown token in Location block: " + key);
    }
    return (loc);
}


// converts "2k" / "5M" / "1024" in bytes
size_t ConfigParser::parseSize(const string &value)
{
    if (value.empty())
        throw ParseException("empty size value in config");

    char suffix = value[value.size() - 1];
    string digits = value;
    size_t multiplier = 1;

    if (suffix == 'k' || suffix == 'K')
    {
        multiplier = 1024;
        digits = value.substr(0, value.size() - 1);
    }
    else if (suffix == 'm' || suffix == 'M')
    {
        multiplier = 1024 * 1024;
        digits = value.substr(0, value.size() - 1);
    }

    for (size_t c = 0; c < digits.size(); ++c)
        if (!std::isdigit(static_cast<unsigned char>(digits[c])))
            throw ParseException("invalid client_max_body_size value: " + value);

    return (static_cast<size_t>(std::atol(digits.c_str())) * multiplier);
}

bool ConfigParser::isValidPort(const string &value)
{
    if (value.empty())
        return false;
    for (size_t c = 0; c < value.size(); ++c)
        if (!std::isdigit(static_cast<unsigned char>(value[c])))
            return false;
    int port = std::atoi(value.c_str());
    return (port > 0 && port <= 65535);
}

bool ConfigParser::isValidIPv4(const string &value)
{
    std::istringstream iss(value);
    string octet;
    int count = 0;

    while (std::getline(iss, octet, '.'))
    {
        if (octet.empty() || octet.size() > 3)
            return false;
        for (size_t c = 0; c < octet.size(); ++c)
            if (!std::isdigit(static_cast<unsigned char>(octet[c])))
                return false;
        int n = std::atoi(octet.c_str());
        if (n < 0 || n > 255)
            return false;
        ++count;
    }
    return (count == 4);
}