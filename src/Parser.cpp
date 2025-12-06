#include "Parser.hpp"
#include <iostream>

Parser::Parser(Scanner* scanner) : scanner(scanner), currentTokenCode(0)
{
}

Parser::~Parser()
{
}

void Parser::nextToken()
{
    currentTokenCode = scanner->scan(currentToken);
}

void Parser::expect(uint16_t tokenCode, const std::string& message)
{
    if (currentTokenCode != tokenCode)
    {
        error(message + ", найден '" + currentToken + "'");
    }
    nextToken();
}

void Parser::error(const std::string& message)
{
    std::cerr << "\nСинтаксическая ошибка: " << message << std::endl;
    std::cerr << scanner->getCurrentLineText() << std::endl;
    throw std::runtime_error("Parsing error");
}

void Parser::parse()
{
    try
    {
        nextToken();
        Program();
        
        if (currentTokenCode != TEnd)
        {
            error("Ожидался конец программы");
        }
        
        std::cout << "\nСинтаксический анализ завершен успешно, ошибок необнаружено" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Анализ прерван из-за ошибок" << std::endl;
    }
}

void Parser::Program()
{
    GlobalDescriptions();
    
    expect(TInt, "Ожидался тип 'int' перед main");
    expect(TMain, "Ожидалось 'main'");
    expect(TLB, "Ожидалась '(' после main");
    expect(TRB, "Ожидалась ')' после main");
    
    CompositeOperator();
}

void Parser::GlobalDescriptions()
{
    while (currentTokenCode == TClass || 
           currentTokenCode == TInt || 
           currentTokenCode == TDouble ||
           currentTokenCode == TId)
    {
        if (currentTokenCode == TInt)
        {
            uint32_t savedPos = scanner->getPos();
            std::string savedToken = currentToken;
            uint16_t savedCode = currentTokenCode;
            
            nextToken();
            if (currentTokenCode == TMain)
            {
                scanner->setPos(savedPos);
                currentToken = savedToken;
                currentTokenCode = savedCode;
                return;
            }
            scanner->setPos(savedPos);
            currentToken = savedToken;
            currentTokenCode = savedCode;
        }
        
        Description();
    }
}

void Parser::Description()
{
    if (currentTokenCode == TClass)
    {
        ClassDesc();
    }
    else if (currentTokenCode == TInt || currentTokenCode == TDouble || currentTokenCode == TId)
    {
        ConstValueDesc();
    }
    else
    {
        error("Ожидалось описание класса или константы");
    }
}

void Parser::ClassDesc()
{
    expect(TClass, "Ожидалось 'class'");
    expect(TId, "Ожидался идентификатор класса");
    expect(TLFB, "Ожидалась '{'");
    
    ClassBody();
    
    expect(TRFB, "Ожидалась '}'");
    expect(TSemicolon, "Ожидалась ';' после определения класса");
}

void Parser::ClassBody()
{
    while (currentTokenCode == TInt || 
           currentTokenCode == TDouble || 
           currentTokenCode == TId)
    {
        MemberDeclaration();
    }
}

void Parser::MemberDeclaration()
{
    uint32_t savedPos = scanner->getPos();
    std::string savedToken = currentToken;
    uint16_t savedCode = currentTokenCode;
    
    Type();
    expect(TId, "Ожидался идентификатор члена класса");
    
    if (currentTokenCode == TLB)
    {
        scanner->setPos(savedPos);
        currentToken = savedToken;
        currentTokenCode = savedCode;
        Method();
    }
    else if (currentTokenCode == TSemicolon)
    {
        scanner->setPos(savedPos);
        currentToken = savedToken;
        currentTokenCode = savedCode;
        Field();
    }
    else
    {
        error("Ожидалась '(' для метода или ';' для поля");
    }
}

void Parser::Method()
{
    Type();
    expect(TId, "Ожидался идентификатор метода");
    expect(TLB, "Ожидалась '(' после имени метода");
    expect(TRB, "Ожидалась ')' - методы без параметров");
    
    CompositeOperator();
}

void Parser::Field()
{
    Type();
    expect(TId, "Ожидался идентификатор поля");
    expect(TSemicolon, "Ожидалась ';' после поля");
}

void Parser::Type()
{
    if (currentTokenCode == TInt || 
        currentTokenCode == TDouble || 
        currentTokenCode == TId)
    {
        nextToken();
    }
    else
    {
        error("Ожидался тип (int, double или идентификатор класса)");
    }
}

void Parser::ConstValueDesc()
{
    Type();
    expect(TId, "Ожидался идентификатор константы");
    expect(TEval, "Ожидался '=' для инициализации константы");
    Const();
    expect(TSemicolon, "Ожидалась ';' после описания константы");
}

void Parser::Const()
{
    if (currentTokenCode == TConstInt || currentTokenCode == TConstDouble)
    {
        nextToken();
    }
    else
    {
        error("Ожидалась константа");
    }
}

void Parser::CompositeOperator()
{
    expect(TLFB, "Ожидалась '{'");
    OperatorsList();
    expect(TRFB, "Ожидалась '}'");
}

void Parser::OperatorsList()
{
    while (currentTokenCode != TRFB && currentTokenCode != TEnd)
    {
        Operator();
    }
}

void Parser::Operator()
{
    if (currentTokenCode == TSemicolon)
    {
        nextToken();
    }
    else if (currentTokenCode == TLFB)
    {
        CompositeOperator();
    }
    else if (currentTokenCode == TWhile)
    {
        While();
    }
    else if (currentTokenCode == TReturn)
    {
        Return();
    }
    else if (currentTokenCode == TInt || 
             currentTokenCode == TDouble || 
             currentTokenCode == TId)
    {
        Statement();
    }
    else
    {
        error("Ожидался оператор");
    }
}

void Parser::Statement()
{
    if (currentTokenCode == TInt || currentTokenCode == TDouble)
    {
        Type();
        expect(TId, "Ожидался идентификатор переменной");
        
        if (currentTokenCode == TEval)
        {
            nextToken();
            Expression();
        }
        
        expect(TSemicolon, "Ожидалась ';' или '.'");
    }
    else if (currentTokenCode == TId)
    {
        uint32_t savedPos = scanner->getPos();
        std::string savedToken = currentToken;
        uint16_t savedCode = currentTokenCode;
        
        nextToken();
        
        if (currentTokenCode == TId)
        {
            scanner->setPos(savedPos);
            currentToken = savedToken;
            currentTokenCode = savedCode;
            
            Type();
            expect(TId, "Ожидался идентификатор переменной");
            
            if (currentTokenCode == TEval)
            {
                nextToken();
                Expression();
            }
            
            expect(TSemicolon, "Ожидалась ';'");
        }
        else if (currentTokenCode == TEval || 
                 currentTokenCode == TPlusEq || 
                 currentTokenCode == TMinusEq || 
                 currentTokenCode == TMultEq || 
                 currentTokenCode == TDivEq || 
                 currentTokenCode == TModEq) 
        {
            nextToken();
            Expression();
            expect(TSemicolon, "Ожидалась ';'");
        }
        else
        {
            error("Ожидался оператор присваивания или идентификатор");
        }
    }
    else
    {
        error("Ожидался тип или идентификатор");
    }
}

void Parser::Assign()
{
    if (currentTokenCode == TInt || currentTokenCode == TDouble)
    {
        Type();
        expect(TId, "Ожидался идентификатор переменной");
        expect(TEval, "Ожидался '='");
        Expression();
        expect(TSemicolon, "Ожидалась ';'");
    }
    else if (currentTokenCode == TId)
    {
        nextToken();
        expect(TEval, "Ожидался '='");
        Expression();
        expect(TSemicolon, "Ожидалась ';'");
    }
    else
    {
        error("Ожидалось присваивание");
    }
}

void Parser::While()
{
    expect(TWhile, "Ожидалось 'while'");
    expect(TLB, "Ожидалась '(' после while");
    Expression();
    expect(TRB, "Ожидалась ')' после условия");
    Operator();
}

void Parser::Return()
{
    expect(TReturn, "Ожидалось 'return'");
    Expression();
    expect(TSemicolon, "Ожидалась ';' после return");
}

void Parser::Expression()
{
    Sum();
    
    if (currentTokenCode == TL || currentTokenCode == TG || 
        currentTokenCode == TLE || currentTokenCode == TGE ||
        currentTokenCode == TNotEq || currentTokenCode == TEq)
    {
        nextToken();
        Sum();
    }
}

void Parser::Sum()
{
    Mult();
    
    while (currentTokenCode == TPlus || currentTokenCode == TMinus)
    {
        nextToken();
        Mult();
    }
}

void Parser::Mult()
{
    Unary();
    
    while (currentTokenCode == TMult || 
           currentTokenCode == TDiv || 
           currentTokenCode == TMod)
    {
        nextToken();
        Unary();
    }
}

void Parser::Unary()
{
    if (currentTokenCode == TPlus || currentTokenCode == TMinus)
    {
        nextToken();
    }
    BaseExp();
}

void Parser::BaseExp()
{
    if (currentTokenCode == TId)
    {
        uint32_t savedPos = scanner->getPos();
        std::string savedToken = currentToken;
        uint16_t savedCode = currentTokenCode;
        
        nextToken();
        if (currentTokenCode == TPoint)
        {
            scanner->setPos(savedPos);
            currentToken = savedToken;
            currentTokenCode = savedCode;
            MethodCall();
        }
    }
    else if (currentTokenCode == TConstInt || currentTokenCode == TConstDouble)
    {
        Const();
    }
    else if (currentTokenCode == TLB)
    {
        nextToken();
        Expression();
        expect(TRB, "Ожидалась ')' после выражения");
    }
    else
    {
        error("Ожидался идентификатор, константа или '('");
    }
}

void Parser::MethodCall()
{
    expect(TId, "Ожидался идентификатор объекта");
    expect(TPoint, "Ожидалась '.' для вызова метода");
    expect(TId, "Ожидался идентификатор метода");
    expect(TLB, "Ожидалась '(' для вызова метода");
    expect(TRB, "Ожидалась ')' - методы без параметров");
}
