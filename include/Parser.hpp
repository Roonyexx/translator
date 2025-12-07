#pragma once

#include <string>
#include <cstdint>

#include "Scanner.hpp"
#include "Tree.hpp"

class Parser {
public:
    Parser(Scanner* scanner);
    ~Parser();

    void parse();

private:
    Scanner* scanner;
    std::string currentToken;
    uint16_t currentTokenCode;
    PrimitiveDataType lastType;
    std::string lastTypeName;
    PrimitiveDataType exprType;
    TData exprValue;

    void nextToken();
    void expect(uint16_t tokenCode, const std::string& message);
    std::string expectId(const std::string& message);
    void error(const std::string& message);
    bool compatibleAssign(PrimitiveDataType l, PrimitiveDataType r);
    void resetExpr();

    void printAssignment(const Node* dest,
                         const std::string& fullName = std::string());
    void assignValue(Node* dest, PrimitiveDataType destType,
                     const TData& srcValue);

    Tree* parseDesignator(bool allowMethodCall,
                          bool& isMethodCall,
                          std::string& fullName);

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
