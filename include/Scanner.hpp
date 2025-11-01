#pragma once 
#include <string>
#include <unordered_map>

#include "TokenType.hpp"

class Scanner
{
public:
    Scanner(const std::string& filename);

    uint16_t scan(std::string& token);

private:
    const std::unordered_map<std::string, int> keywords {
        { "int", TInt },
        { "double", TDouble },
        { "void", TVoid },
        { "class", TClass },
        { "while", TWhile },
        { "return", TReturn },
        { "main", TMain }
    };

    std::string programText;
    uint32_t pos;

    void getTextFromFile(const std::string& filname);
    bool isDigit(const char& ch);
    bool isLetter(const char& ch);
    bool isKeyword(const std::string& token);
    void printError(const std::string& error, const std::string& token);
};