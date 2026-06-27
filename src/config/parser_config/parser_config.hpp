

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

class Tokenizer {

  
    public:
    Tokenizer(const std::string &input) : _input(input), _pos(0){};
    ~Tokenizer{};  
    std::vector<std::string> tokenize();


}

std::vector<std::string> Tokenizer::tokenize(const std::string& input)
{
    std::vector<std::string> token;
    std::string std_space = _add_space(input);
    //split strings
    
    std::istringstream mtx_std(std_space);
    std::string tokens;

    while(mtx_std >> tokens)
        token.push_back(tokens);

    return token;
}