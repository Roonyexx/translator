#pragma once

#include <string>
#include <unordered_map>

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

    bool flagInterpret;
    bool flagReturn;

    PrimitiveDataType returnType;
    TData returnValue;

    Tree* mainTree;
    uint32_t mainBodyPos;

    std::unordered_map<Tree*, uint32_t> methodBodyPos;

    void nextToken();

    void expect(uint16_t tokenCode, const std::string& message);
    std::string expectId(const std::string& message);

    [[noreturn]] void error(const std::string& message);

    bool compatibleAssign(PrimitiveDataType l, PrimitiveDataType r);

    uint32_t getUK() const;
    void setUK(uint32_t uk);

    void debugFlag(const std::string& where);
    void debugEvent(const std::string& msg);

    void resetExpr();

    void printAssignment(const Node* dest, const std::string& fullName = std::string());
    void assignValue(Node* dest, PrimitiveDataType destType, const TData& srcValue);

    void debugValue(const char* label, const TData& v, PrimitiveDataType t);

    TData evalBinary(uint16_t op,
                     const TData& l, PrimitiveDataType lt,
                     const TData& r, PrimitiveDataType rt,
                     PrimitiveDataType& resultType);

    TData execMethod(Tree* methodNode, const std::string& fullName);

    Tree* parseDesignator(bool allowMethodCall, bool& isMethodCall, std::string& fullName);

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
    void Statement();

    void While();
    void Return();

    void Expression();
    void Sum();
    void Mult();
    void Unary();
    void BaseExp();

    void Assign();
    void MethodCall();
};
