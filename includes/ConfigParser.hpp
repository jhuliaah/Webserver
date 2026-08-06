
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

    static std::vector<ParsedServer> parseTokens(const std::vector<string> &tokens);
    static ParsedServer parseServerBlock(const std::vector<string> &tokens, size_t &i);
    static ParsedLocation parseLocationBlock(const std::vector<string> &tokens, size_t &i);
    static void expect(const std::vector<string> &tokens, size_t i, const string &value);
    static size_t parseSize(const string &value);
    static bool isValidPort(const string &value);
    static bool isValidIPv4(const string &value);


    public:
    class ParseException : public std::exception
    {
        private:
        string _msg;
        
        public:
        ParseException(const string &msg) : _msg(msg){}
        virtual ~ParseException() throw() {}
        virtual const char *what() const throw() { return _msg.c_str();}
    };

    static std::vector<ParsedServer> parse(const string &path); //

};

#endif

