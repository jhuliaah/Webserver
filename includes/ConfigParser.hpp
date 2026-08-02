
#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "ConfigTypes.hpp"
#include <vector>
#include <string>
#include <stdexcept>

typedef std::string string;


class ConfigParser{

    private:
    ConfigParser();
    ~ConfigParser();
    ConfigParser(const ConfigParser &other);
    ConfigParser &operator=(const ConfigParser &other);

    static std::vector<ServerConfig> parseTokens(const std::vector<string> &tokens);
    static ServerConfig parseServerBlock(const std::vector<string> &tokens, size_t &i);
    static location parseLocationBlock(const std::vector<string> &tokens, size_t &i);
    static void expect(const std::vector<string> &tokens, size_t i, const string &value);

    public:
    class ParseException : public std::exception
    {
        private:
        string _msg;
        
        public:
        ParseException(const string &msg) : _msg(msg){}
        virtual ~ParseException() throw {}
        virtual const char *what() const throw() { return _msg.c_str();}
    };

    static std::vector<ServerConfig> parse(const string &path); //

};

#endif

