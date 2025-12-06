#include "Parser.hpp"
#include <iostream>
#include <stdexcept>

Parser::Parser(Scanner* scanner)
    : scanner(scanner),
      currentTokenCode(0),
      lastType(UndefinedType),
      lastTypeName(),
      exprType(UndefinedType)
{
    Node globalNode("global", ObjEmpty, UndefinedType);
    Tree::SetRight(globalNode);
}

Parser::~Parser() {}

void Parser::nextToken()
{
    currentTokenCode = scanner->scan(currentToken);
}

void Parser::expect(uint16_t tokenCode, const std::string& message)
{
    if (currentTokenCode != tokenCode)
        error(message + ", найден '" + currentToken + "'");
    nextToken();
}

std::string Parser::expectId(const std::string& message)
{
    if (currentTokenCode != TId)
        error(message + ", найден '" + currentToken + "'");
    std::string id = currentToken;
    nextToken();
    return id;
}

void Parser::error(const std::string& message)
{
    std::cerr << "\nОшибка: " << message << std::endl;
    try
    {
        std::cerr << scanner->getCurrentLineText() << std::endl;
    }
    catch (...)
    {
    }
    throw std::runtime_error("Parsing error");
}

bool Parser::compatibleAssign(PrimitiveDataType l, PrimitiveDataType r)
{
    if (l == UndefinedType || r == UndefinedType) return false;
    if (l == r) return true;
    if (l == DoubleType && r == IntType) return true;
    return false;
}

void Parser::parse()
{
    try
    {
        nextToken();
        Program();

        if (currentTokenCode != TEnd)
            error("Ожидался конец программы");

        std::cout << "\nАнализ завершён успешно.\n";
        Tree::PrintTree(Tree::getCurrent());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Анализ прерван: " << e.what() << std::endl;
    }
}

void Parser::Program()
{
    GlobalDescriptions();

    expect(TInt, "Ожидался 'int' перед main");
    expect(TMain, "Ожидалось 'main'");
    expect(TLB, "Ожидалась '(' после main");
    expect(TRB, "Ожидалась ')' после main");

    Node mainNode("main", ObjFunc, IntType);
    Tree* mainTree = Tree::SetRight(mainNode);
    Tree::setCurrent(mainTree);

    CompositeOperator();

    Tree::setCurrent(mainTree->getParent());
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
            uint32_t    savedPos   = scanner->getPos();
            std::string savedToken = currentToken;
            uint16_t    savedCode  = currentTokenCode;

            nextToken();
            if (currentTokenCode == TMain)
            {
                scanner->setPos(savedPos);
                currentToken     = savedToken;
                currentTokenCode = savedCode;
                return;
            }

            scanner->setPos(savedPos);
            currentToken     = savedToken;
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
    std::string className = expectId("Ожидался идентификатор класса");
    expect(TLFB, "Ожидалась '{'");

    if (!checkDuplicateId(className))
        throw std::runtime_error("Семантическая ошибка");

    Node cls(className, ObjClass, UndefinedType);
    Tree* clsNode = Tree::SetRight(cls);
    Tree::setCurrent(clsNode);

    ClassBody();

    Tree::setCurrent(clsNode->getParent());

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
    uint32_t    savedPos   = scanner->getPos();
    std::string savedToken = currentToken;
    uint16_t    savedCode  = currentTokenCode;

    Type();
    std::string memberId = expectId("Ожидался идентификатор члена класса");

    if (currentTokenCode == TLB)
    {
        scanner->setPos(savedPos);
        currentToken     = savedToken;
        currentTokenCode = savedCode;
        Method();
    }
    else if (currentTokenCode == TSemicolon)
    {
        scanner->setPos(savedPos);
        currentToken     = savedToken;
        currentTokenCode = savedCode;
        Field();
    }
    else
    {
        error("Ожидался '(' или ';' при объявлении члена класса");
    }
}

void Parser::Method()
{
    Type();
    std::string methodName = expectId("Ожидался идентификатор метода");

    if (!checkDuplicateId(methodName))
        throw std::runtime_error("Семантическая ошибка: дублирование метода");

    expect(TLB, "Ожидалась '(' после имени метода");
    expect(TRB, "Ожидалась ')' - методы без параметров");

    Node m(methodName, ObjMethod, lastType);
    Tree* mNode = Tree::SetRight(m);
    Tree::setCurrent(mNode);

    CompositeOperator();

    Tree::setCurrent(mNode->getParent());
}

void Parser::Field()
{
    Type();
    std::string fieldName = expectId("Ожидался идентификатор поля");

    if (!checkDuplicateId(fieldName))
        throw std::runtime_error("Семантическая ошибка: дублирование поля");

    expect(TSemicolon, "Ожидалась ';' после поля");

    if (lastType == UndefinedType && !lastTypeName.empty())
    {
        Node f(fieldName, ObjVar, UndefinedType, false, lastTypeName);
        Tree::SetRight(f);
    }
    else
    {
        Node f(fieldName, ObjVar, lastType);
        Tree::SetRight(f);
    }
}

void Parser::Type()
{
    if (currentTokenCode == TInt)
    {
        lastType = IntType;
        lastTypeName.clear();
        nextToken();
    }
    else if (currentTokenCode == TDouble)
    {
        lastType = DoubleType;
        lastTypeName.clear();
        nextToken();
    }
    else if (currentTokenCode == TId)
    {
        lastType     = UndefinedType;
        lastTypeName = currentToken;
        nextToken();
    }
    else
    {
        error("Ожидался тип (int, double или идентификатор)");
    }
}

void Parser::ConstValueDesc()
{
    Type();
    std::string constName = expectId("Ожидался идентификатор константы");

    if (!checkDuplicateId(constName))
        throw std::runtime_error("Семантическая ошибка: дублирование константы");

    expect(TEval, "Ожидался '=' для инициализации константы");
    Const();
    expect(TSemicolon, "Ожидалась ';' после константы");

    Node c(constName, ObjConst, lastType);
    Tree::SetRight(c);
}

void Parser::Const()
{
    if (currentTokenCode == TConstInt)
    {
        exprType = IntType;
        nextToken();
    }
    else if (currentTokenCode == TConstDouble)
    {
        exprType = DoubleType;
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

    Tree::semIn();
    OperatorsList();
    Tree::semOut();

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
        PrimitiveDataType declType =
            (currentTokenCode == TInt ? IntType : DoubleType);
        nextToken();
        std::string varName = expectId("Ожидался идентификатор переменной");

        if (!checkDuplicateId(varName))
            throw std::runtime_error("Семантическая ошибка: дублирование переменной");

        bool inited = false;

        if (currentTokenCode == TEval)
        {
            nextToken();
            Expression();
            if (!compatibleAssign(declType, exprType))
            {
                std::cerr << "Семантическая ошибка: несовместимые типы при инициализации '"
                          << varName << "'\n";
                throw std::runtime_error("Семантическая ошибка");
            }
            inited = true;
        }

        Node v(varName, ObjVar, declType, inited, "");
        Tree::SetRight(v);

        expect(TSemicolon, "Ожидалась ';' после объявления");
        return;
    }

    if (currentTokenCode == TId)
    {
        uint32_t    savedPos   = scanner->getPos();
        std::string savedToken = currentToken;
        uint16_t    savedCode  = currentTokenCode;

        nextToken();

        if (currentTokenCode == TId)
        {
            std::string typeName = savedToken;
            std::string varName  = currentToken;
            nextToken();

            if (!checkDuplicateId(varName))
                throw std::runtime_error("Семантическая ошибка: дублирование переменной");

            PrimitiveDataType declType    = UndefinedType;
            std::string       declTypeName = typeName;

            Tree* classDef = Tree::FindGlobal(declTypeName);
            if (!classDef || !classDef->getNode() || classDef->getNode()->objType != ObjClass)
            {
                std::cerr << "Семантическая ошибка: тип '" << declTypeName
                          << "' не найден как класс\n";
                throw std::runtime_error("Семантическая ошибка");
            }

            if (currentTokenCode == TEval)
            {
                nextToken();
                Expression();
                std::cerr << "Семантическая ошибка: инициализация объектного типа литералом запрещена\n";
                throw std::runtime_error("Семантическая ошибка");
            }

            Node v(varName, ObjVar, declType, false, declTypeName);
            Tree::SetRight(v);

            expect(TSemicolon, "Ожидалась ';' после объявления");
            return;
        }
        else
        {
            scanner->setPos(savedPos);
            currentToken     = savedToken;
            currentTokenCode = savedCode;
        }

        std::string ident = currentToken;
        nextToken();

        if (currentTokenCode == TEval ||
            currentTokenCode == TPlusEq || currentTokenCode == TMinusEq ||
            currentTokenCode == TMultEq || currentTokenCode == TDivEq ||
            currentTokenCode == TModEq)
        {
            if (!checkId(ident))
                throw std::runtime_error("Семантическая ошибка: использование необъявленного идентификатора");

            Tree* leftNode = Tree::getCurrent()->FindUp(ident);
            if (!checkLValue(leftNode))
            {
                std::cerr << "Семантическая ошибка: слева от присваивания должно быть изменяемое значение\n";
                throw std::runtime_error("Семантическая ошибка");
            }

            nextToken();
            Expression();

            PrimitiveDataType leftType = leftNode->getNode()->datType;
            if (leftType == UndefinedType && !leftNode->getNode()->typeName.empty())
            {
                std::cerr << "Семантическая ошибка: присваивание объектному типу из числового выражения запрещено\n";
                throw std::runtime_error("Семантическая ошибка");
            }

            if (!compatibleAssign(leftType, exprType))
            {
                std::cerr << "Семантическая ошибка: несовместимые типы при присваивании '" << ident << "'\n"; 
                throw std::runtime_error("Семантическая ошибка");
            }

            expect(TSemicolon, "Ожидалась ';' после присваивания");
            return;
        }
        else if (currentTokenCode == TPoint)
        {
            currentToken     = ident;
            currentTokenCode = TId;
            BaseExp();
            expect(TSemicolon, "Ожидалась ';' после вызова/выражения");
            return;
        }
        else
        {
            error("Неподдерживаемая конструкция после идентификатора");
        }
    }

    error("Ожидался тип или идентификатор");
}

void Parser::Assign()
{
    error("Assign() не используется в данной реализации");
}

void Parser::While()
{
    expect(TWhile, "Ожидалось 'while'");
    expect(TLB, "Ожидалась '(' после while");
    Expression();
    if (!(exprType == IntType || exprType == DoubleType))
    {
        std::cerr << "Семантическая ошибка: условие while должно быть числовым\n";
        throw std::runtime_error("Семантическая ошибка");
    }
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
        exprType = IntType;
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
           currentTokenCode == TDiv  ||
           currentTokenCode == TMod)
    {
        nextToken();
        Unary();
    }
}

void Parser::Unary()
{
    if (currentTokenCode == TPlus || currentTokenCode == TMinus)
        nextToken();
    BaseExp();
}

void Parser::BaseExp()
{
    if (currentTokenCode == TId)
    {
        std::string name = currentToken;

        if (!checkId(name))
            throw std::runtime_error("Семантическая ошибка: использование необъявленного идентификатора");

        Tree* node = Tree::getCurrent()->FindUp(name);
        if (!node)
            throw std::runtime_error("Семантическая ошибка: внутренний поиск идентификатора провалился");

        nextToken();

        if (currentTokenCode == TPoint)
        {
            nextToken();
            std::string method = expectId("Ожидался идентификатор метода после '.'");

            Tree* classNode = nullptr;

            if (node->getNode()->objType == ObjClass)
            {
                classNode = node;
            }
            else if (node->getNode()->objType == ObjVar && !node->getNode()->typeName.empty())
            {
                classNode = Tree::FindGlobal(node->getNode()->typeName);
            }
            else
            {
                std::cerr << "Семантическая ошибка: вызов метода у '" << name << "' невозможен\n";
                throw std::runtime_error("Семантическая ошибка");
            }

            if (!classNode)
            {
                std::cerr << "Семантическая ошибка: класс '"
                          << (node->getNode()->typeName.empty() ? name : node->getNode()->typeName)
                          << "' не найден\n";
                throw std::runtime_error("Семантическая ошибка");
            }

            Tree* methodNode = checkMethod(classNode, method);
            if (!methodNode)
            {
                std::cerr << "Семантическая ошибка: метод '" << method
                          << "' отсутствует в классе\n";
                throw std::runtime_error("Семантическая ошибка");
            }

            exprType = methodNode->getNode()->datType;

            expect(TLB, "Ожидалась '(' при вызове метода");
            expect(TRB, "Ожидалась ')' при вызове метода");
        }
        else
        {
            exprType = node->getNode()->datType;
        }

        return;
    }
    else if (currentTokenCode == TConstInt)
    {
        exprType = IntType;
        nextToken();
        return;
    }
    else if (currentTokenCode == TConstDouble)
    {
        exprType = DoubleType;
        nextToken();
        return;
    }
    else if (currentTokenCode == TLB)
    {
        nextToken();
        Expression();
        expect(TRB, "Ожидалась ')'");
        return;
    }

    error("Ожидался идентификатор, константа или '('");
}

void Parser::MethodCall()
{
    std::string obj = expectId("Ожидался идентификатор объекта");
    expect(TPoint, "Ожидалась '.'");
    std::string method = expectId("Ожидался идентификатор метода");
    expect(TLB, "Ожидалась '('");
    expect(TRB, "Ожидалась ')'");
}
