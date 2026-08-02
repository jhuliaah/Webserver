
#include "../../includes/Tokenizer.hpp"
#include <sstream>

std::vector<string> Tokenizer::tokenize(const string &content)
{
    string spaced;
    spaced.reserve(content.size() * 2);

    for (size_t i = 0; i <content.size(); ++i)
    {
        char c = content[i];
        if (c == '{' || c == '}' || c == ';')
        {
            spaced += ' ';
            spaced += c;
            spaced += ' ';
        }
        else if (c == '#') //for coments
        {
            while (i <content.size() && content[i] != '\n')
             ++i;
        }
        else
            spaced += c;
    }

    std::vector<string> tokens;
    std::istringstream iss(spaced);
    string  token;
    while (iss >> token)
        tokens.push_back(token);

    return (tokens);
}