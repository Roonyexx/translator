#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "TokenType.hpp"

class Scanner
{
public:
    Scanner(const std::string& filename);
    uint16_t scan(std::string& token);
    
    uint32_t getPos() const { return pos; }
    void setPos(uint32_t newPos) { pos = newPos; }
    
    uint32_t getLine() const { return currentLine; }
    uint32_t getTokenStartPos() const { return tokenStartPos; } 
    std::string getCurrentLineText() const;

private:
    const std::unordered_map<std::string, uint16_t> keywords {
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
    uint32_t currentLine;
    std::vector<uint32_t> lineStarts;
    uint32_t tokenStartPos;
    
    void getTextFromFile(const std::string& filname);
    void buildLineIndex();
    bool isDigit(const char& ch);
    bool isLetter(const char& ch);
    bool isKeyword(const std::string& token);
    void printError(const std::string& error, const std::string& token);
};
