#pragma once

#include <string>
#include "Scanner.hpp"

class Parser
{
public:
    Parser(Scanner* scanner);
    ~Parser();
    
    void parse();
    
private:
    Scanner* scanner;
    std::string currentToken;
    uint16_t currentTokenCode;
    
    void nextToken();
    void expect(uint16_t tokenCode, const std::string& message);
    void error(const std::string& message);
    
    void Program();
    void GlobalDescriptions();
    void Description();
    void ClassDesc();
    void ClassBody();
    void MemberDeclaration();
    void Method();
    void Field();
    void Type();
    void ConstValueDesc();
    void Const();
    void CompositeOperator();
    void OperatorsList();
    void Operator();
    void Assign();
    void Statement();
    void While();
    void Return();
    void Expression();
    void Sum();
    void Mult();
    void Unary();
    void BaseExp();
    void MethodCall();
};
