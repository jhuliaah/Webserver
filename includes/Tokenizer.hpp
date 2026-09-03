
#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>
#include <vector>
typedef std::string string;

class Tokenizer
{

    public:
        static std::vector<string> tokenize(const string &content);

    private:
        Tokenizer();
        ~Tokenizer();
        Tokenizer(const Tokenizer &other);
        Tokenizer &operator=(const Tokenizer &other);
};

#endif