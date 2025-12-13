#include "Parser.hpp"

#include <iostream>
#include <stdexcept>

static bool condToBool(const TData& v) {
    if (v.dataType == TYPE_INT) return v.dataValue.dataAsInt != 0;
    if (v.dataType == TYPE_DOUBLE) return v.dataValue.dataAsDouble != 0.0;
    return false;
}

uint32_t Parser::getUK() const {
    return scanner->getPos();
}

void Parser::setUK(uint32_t uk) {
    scanner->setPos(uk);
    nextToken();
}

void Parser::debugFlag(const std::string& where) {
    if (!flagInterpret) {
        return;
    }
    std::cout << "[DEBUG] " << where << ": flagInterpret="
              << (flagInterpret ? "UP" : "DOWN")
              << ", flagReturn=" << (flagReturn ? "TRUE" : "FALSE")
              << std::endl;
}

void Parser::debugEvent(const std::string& msg) {
    if (!flagInterpret) {
        return;
    }
    std::cout << "[DEBUG] " << msg << std::endl;
}

void Parser::resetExpr() {
    exprType = UndefinedType;
    exprValue.dataType = TYPE_UNKNOWN;
    exprValue.dataValue.dataAsInt = 0;
}

void Parser::printAssignment(const Node* dest, const std::string& fullName) {
    if (!dest) {
        return;
    }
    const std::string& name = fullName.empty() ? dest->id : fullName;
    std::cout << name << " = ";
    switch (dest->data.dataType) {
        case TYPE_INT:
            std::cout << dest->data.dataValue.dataAsInt;
            break;
        case TYPE_DOUBLE:
            std::cout << dest->data.dataValue.dataAsDouble;
            break;
        default:
            std::cout << "";
            break;
    }
    std::cout << std::endl;
}

void Parser::assignValue(Node* dest, PrimitiveDataType destType, const TData& srcValue) {
    if (!dest) {
        return;
    }

    switch (destType) {
        case IntType:
            dest->data.dataType = TYPE_INT;
            if (srcValue.dataType == TYPE_INT) {
                dest->data.dataValue.dataAsInt = srcValue.dataValue.dataAsInt;
            } else if (srcValue.dataType == TYPE_DOUBLE) {
                std::cout << "приведение типа (double -> int) при присваивании '"
                          << dest->id << "'" << std::endl;
                dest->data.dataValue.dataAsInt =
                    static_cast<int>(srcValue.dataValue.dataAsDouble);
            } else {
                dest->data.dataValue.dataAsInt = 0;
            }
            break;

        case DoubleType:
            dest->data.dataType = TYPE_DOUBLE;
            if (srcValue.dataType == TYPE_DOUBLE) {
                dest->data.dataValue.dataAsDouble = srcValue.dataValue.dataAsDouble;
            } else if (srcValue.dataType == TYPE_INT) {
                std::cout << "приведение типа (int -> double) при присваивании '"
                          << dest->id << "'" << std::endl;
                dest->data.dataValue.dataAsDouble =
                    static_cast<double>(srcValue.dataValue.dataAsInt);
            } else {
                dest->data.dataValue.dataAsDouble = 0.0;
            }
            break;

        default:
            dest->data.dataType = TYPE_UNKNOWN;
            break;
    }

    dest->isInitialized = true;
}

void Parser::debugValue(const char* label, const TData& v, PrimitiveDataType t) {
    if (!flagInterpret) {
        return;
    }
    std::cout << label << ": ";
    switch (v.dataType) {
        case TYPE_INT:
            std::cout << v.dataValue.dataAsInt << " (int)";
            break;
        case TYPE_DOUBLE:
            std::cout << v.dataValue.dataAsDouble << " (double)";
            break;
        default:
            std::cout << "";
            break;
    }
    if (t == IntType) {
        std::cout << " [IntType]";
    } else if (t == DoubleType) {
        std::cout << " [DoubleType]";
    }
    std::cout << std::endl;
}

TData Parser::evalBinary(uint16_t op,
                         const TData& l, PrimitiveDataType lt,
                         const TData& r, PrimitiveDataType rt,
                         PrimitiveDataType& resultType) {
    TData res;
    res.dataType = TYPE_UNKNOWN;
    resultType = UndefinedType;

    auto isCompare = [&](uint16_t token) -> bool {
        return token == TL || token == TG ||
               token == TLE || token == TGE ||
               token == TEq || token == TNotEq;
    };

    // В режиме проектирования значения не вычисляем и отладку не печатаем.
    if (!flagInterpret) {
        if (isCompare(op)) {
            resultType = IntType;
            res.dataType = TYPE_INT;
            res.dataValue.dataAsInt = 0;
            return res;
        }

        resultType = (lt == DoubleType || rt == DoubleType) ? DoubleType : IntType;
        res.dataType = (resultType == DoubleType) ? TYPE_DOUBLE : TYPE_INT;
        if (res.dataType == TYPE_DOUBLE) res.dataValue.dataAsDouble = 0.0;
        else res.dataValue.dataAsInt = 0;
        return res;
    }

    auto opName = [](uint16_t token) -> const char* {
        switch (token) {
            case TPlus:   return "+";
            case TMinus:  return "-";
            case TMult:   return "*";
            case TDiv:    return "/";
            case TMod:    return "%";
            case TL:      return "<";
            case TG:      return ">";
            case TLE:     return "<=";
            case TGE:     return ">=";
            case TEq:     return "==";
            case TNotEq:  return "!=";
            default:      return "?";
        }
    };

    std::cout << "[DEBUG] Операция: ";
    if (l.dataType == TYPE_INT) std::cout << l.dataValue.dataAsInt << " (int) ";
    else if (l.dataType == TYPE_DOUBLE) std::cout << l.dataValue.dataAsDouble << " (double) ";

    std::cout << opName(op) << " ";

    if (r.dataType == TYPE_INT) std::cout << r.dataValue.dataAsInt << " (int) ";
    else if (r.dataType == TYPE_DOUBLE) std::cout << r.dataValue.dataAsDouble << " (double) ";

    if (isCompare(op)) {
        bool cmp = false;

        double lv = (l.dataType == TYPE_DOUBLE)
                        ? l.dataValue.dataAsDouble
                        : static_cast<double>(l.dataValue.dataAsInt);
        double rv = (r.dataType == TYPE_DOUBLE)
                        ? r.dataValue.dataAsDouble
                        : static_cast<double>(r.dataValue.dataAsInt);

        switch (op) {
            case TL:     cmp = (lv <  rv); break;
            case TG:     cmp = (lv >  rv); break;
            case TLE:    cmp = (lv <= rv); break;
            case TGE:    cmp = (lv >= rv); break;
            case TEq:    cmp = (lv == rv); break;
            case TNotEq: cmp = (lv != rv); break;
        }

        res.dataType = TYPE_INT;
        res.dataValue.dataAsInt = cmp ? 1 : 0;
        resultType = IntType;

        std::cout << " -> " << res.dataValue.dataAsInt << " (int, compare)" << std::endl;
        return res;
    }

    bool useDouble =
        (l.dataType == TYPE_DOUBLE || r.dataType == TYPE_DOUBLE ||
         lt == DoubleType || rt == DoubleType);

    if (useDouble) {
        double lv = (l.dataType == TYPE_DOUBLE) ? l.dataValue.dataAsDouble
                                                : static_cast<double>(l.dataValue.dataAsInt);
        double rv = (r.dataType == TYPE_DOUBLE) ? r.dataValue.dataAsDouble
                                                : static_cast<double>(r.dataValue.dataAsInt);

        double out = 0.0;
        switch (op) {
            case TPlus:  out = lv + rv; break;
            case TMinus: out = lv - rv; break;
            case TMult:  out = lv * rv; break;
            case TDiv:
                if (rv == 0.0) {
                    std::cerr << "Warning: division by zero (double)" << std::endl;
                }
                out = (rv != 0.0) ? (lv / rv) : 0.0;
                break;
            case TMod:
                std::cerr << "Warning: operator % for double, casting to int" << std::endl;
                out = static_cast<int>(lv) % static_cast<int>(rv);
                break;
            default:
                std::cerr << "Semantic error: unknown binary operation" << std::endl;
                break;
        }

        res.dataType = TYPE_DOUBLE;
        res.dataValue.dataAsDouble = out;
        resultType = DoubleType;

        std::cout << " -> " << out << " (double)" << std::endl;
        return res;
    }

    int lv = l.dataValue.dataAsInt;
    int rv = r.dataValue.dataAsInt;
    int out = 0;

    switch (op) {
        case TPlus:  out = lv + rv; break;
        case TMinus: out = lv - rv; break;
        case TMult:  out = lv * rv; break;
        case TDiv:
            if (rv == 0) {
                std::cerr << "Warning: division by zero (int)" << std::endl;
            }
            out = (rv != 0) ? (lv / rv) : 0;
            break;
        case TMod:
            if (rv == 0) {
                std::cerr << "Warning: modulo by zero" << std::endl;
                out = 0;
            } else {
                out = lv % rv;
            }
            break;
        default:
            std::cerr << "Semantic error: unknown binary operation" << std::endl;
            break;
    }

    res.dataType = TYPE_INT;
    res.dataValue.dataAsInt = out;
    resultType = IntType;

    std::cout << " -> " << out << " (int)" << std::endl;
    return res;
}

Parser::Parser(Scanner* scanner)
    : scanner(scanner),
      currentToken(),
      currentTokenCode(0),
      lastType(UndefinedType),
      lastTypeName(),
      exprType(UndefinedType),
      exprValue(),
      flagInterpret(false),
      flagReturn(false),
      returnType(UndefinedType),
      returnValue(),
      mainTree(nullptr),
      mainBodyPos(0),
      methodBodyPos() {
    Node globalNode("global", ObjEmpty, UndefinedType);
    Tree::SetRight(globalNode);
}

Parser::~Parser() {}

void Parser::nextToken() {
    currentTokenCode = scanner->scan(currentToken);
}

void Parser::expect(uint16_t tokenCode, const std::string& message) {
    if (currentTokenCode != tokenCode) {
        error(message + ", got '" + currentToken + "'");
    }
    nextToken();
}

std::string Parser::expectId(const std::string& message) {
    if (currentTokenCode != TId) {
        error(message + ", got '" + currentToken + "'");
    }
    std::string id = currentToken;
    nextToken();
    return id;
}

void Parser::error(const std::string& message) {
    std::cerr << "\nОшибка: " << message << std::endl;
    try {
        std::cerr << scanner->getCurrentLineText() << std::endl;
    } catch (...) {
    }
    throw std::runtime_error("Parsing error");
}

bool Parser::compatibleAssign(PrimitiveDataType l, PrimitiveDataType r) {
    if (l == UndefinedType || r == UndefinedType) {
        return false;
    }
    if (l == r) {
        return true;
    }
    if (l == DoubleType && r == IntType) {
        return true;
    }
    if (l == IntType && r == DoubleType) {
        return true;
    }
    return false;
}

void Parser::parse() {
    try {
        nextToken();
        Program();
        if (currentTokenCode != TEnd) {
            error("Ожидался конец программы");
        }

        std::cout << "\nАнализ завершён успешно.\n";

        if (mainTree) {
            flagInterpret = true;
            flagReturn = false;
            returnType = IntType;
            returnValue = TData();

            debugEvent("Начинаю выполнение main");

            Tree::setCurrent(mainTree);
            setUK(mainBodyPos);

            Tree::semIn();
            OperatorsList();
            Tree::semOut();

            expect(TRFB, "Ожидалась '}'");

            debugEvent("Завершаю выполнение main");
        }

        Tree::PrintTree(Tree::getCurrent());
    } catch (const std::exception& e) {
        std::cerr << "Анализ прерван: " << e.what() << std::endl;
    }
}

void Parser::Program() {
    GlobalDescriptions();

    expect(TInt,  "Ожидался 'int' перед main");
    expect(TMain, "Ожидалось 'main'");
    expect(TLB,   "Ожидалась '(' после main");
    expect(TRB,   "Ожидалась ')' после main");

    Node mainNode("main", ObjFunc, IntType);
    mainTree = Tree::SetRight(mainNode);
    Tree::setCurrent(mainTree);

    expect(TLFB, "Ожидалась '{'");

    // старт тела main = позиция начала первого токена внутри { ... }
    mainBodyPos = scanner->getTokenStartPos();

    bool saved = flagInterpret;
    flagInterpret = false;

    Tree::semIn();
    OperatorsList();
    Tree::semOut();
    expect(TRFB, "Ожидалась '}'");

    flagInterpret = saved;
    Tree::setCurrent(mainTree->getParent());
}

void Parser::GlobalDescriptions() {
    while (currentTokenCode == TClass ||
           currentTokenCode == TInt   ||
           currentTokenCode == TDouble||
           currentTokenCode == TId) {
        if (currentTokenCode == TInt) {
            uint32_t    savedPos   = scanner->getPos();
            std::string savedToken = currentToken;
            uint16_t    savedCode  = currentTokenCode;

            nextToken();
            if (currentTokenCode == TMain) {
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

void Parser::Description() {
    if (currentTokenCode == TClass) {
        ClassDesc();
    } else if (currentTokenCode == TInt ||
               currentTokenCode == TDouble ||
               currentTokenCode == TId) {
        uint32_t    savedPos   = scanner->getPos();
        std::string savedToken = currentToken;
        uint16_t    savedCode  = currentTokenCode;

        Type();
        (void)expectId("Ожидался идентификатор");

        if (currentTokenCode == TEval) {
            scanner->setPos(savedPos);
            currentToken     = savedToken;
            currentTokenCode = savedCode;
            ConstValueDesc();
        } else if (currentTokenCode == TSemicolon ||
                   currentTokenCode == TComma) {
            scanner->setPos(savedPos);
            currentToken     = savedToken;
            currentTokenCode = savedCode;
            Statement();
        } else {
            error("Ожидалось '=' или ';' в глобальном описании");
        }
    } else {
        error("Ожидалось описание класса или объявления");
    }
}

void Parser::ClassDesc() {
    expect(TClass, "Ожидалось 'class'");
    std::string className = expectId("Ожидался идентификатор класса");
    expect(TLFB, "Ожидалась '{'");

    if (!checkDuplicateId(className)) {
        throw std::runtime_error("Семантическая ошибка");
    }

    Node cls(className, ObjClass, UndefinedType);
    Tree* clsNode = Tree::SetRight(cls);
    Tree::setCurrent(clsNode);

    ClassBody();

    Tree::setCurrent(clsNode->getParent());
    expect(TRFB,      "Ожидалась '}'");
    expect(TSemicolon,"Ожидалась ';' после определения класса");
}

void Parser::ClassBody() {
    while (currentTokenCode == TInt ||
           currentTokenCode == TDouble ||
           currentTokenCode == TId) {
        MemberDeclaration();
    }
}

void Parser::MemberDeclaration() {
    uint32_t    savedPos   = scanner->getPos();
    std::string savedToken = currentToken;
    uint16_t    savedCode  = currentTokenCode;

    Type();
    (void)expectId("Ожидался идентификатор члена класса");

    if (currentTokenCode == TLB) {
        scanner->setPos(savedPos);
        currentToken     = savedToken;
        currentTokenCode = savedCode;
        Method();
    } else if (currentTokenCode == TComma ||
               currentTokenCode == TSemicolon) {
        scanner->setPos(savedPos);
        currentToken     = savedToken;
        currentTokenCode = savedCode;
        Field();
    } else {
        error("Ожидался '(' или ';' при объявлении члена класса");
    }
}

void Parser::Method() {
    Type();
    std::string methodName = expectId("Ожидался идентификатор метода");

    if (!checkDuplicateId(methodName)) {
        throw std::runtime_error("Семантическая ошибка: дублирование метода");
    }

    expect(TLB, "Ожидалась '(' после имени метода");
    expect(TRB, "Ожидалась ')' - методы без параметров");

    Node m(methodName, ObjMethod, lastType);
    Tree* mNode = Tree::SetRight(m);
    Tree::setCurrent(mNode);

    expect(TLFB, "Ожидалась '{'");

    // старт тела метода
    methodBodyPos[mNode] = scanner->getTokenStartPos();

    bool saved = flagInterpret;
    flagInterpret = false;

    Tree::semIn();
    OperatorsList();
    Tree::semOut();

    expect(TRFB, "Ожидалась '}'");

    flagInterpret = saved;
    Tree::setCurrent(mNode->getParent());
}

void Parser::Field() {
    Type();
    std::string fieldName = expectId("Ожидался идентификатор поля");

    while (true) {
        if (!checkDuplicateId(fieldName)) {
            throw std::runtime_error("Семантическая ошибка: дублирование поля");
        }

        if (lastType == UndefinedType && !lastTypeName.empty()) {
            Node f(fieldName, ObjField, UndefinedType, false, lastTypeName);
            Tree::SetRight(f);
        } else {
            Node f(fieldName, ObjField, lastType);
            Tree::SetRight(f);
        }

        if (currentTokenCode == TComma) {
            nextToken();
            fieldName = expectId("Ожидался идентификатор поля");
            continue;
        }

        break;
    }

    expect(TSemicolon, "Ожидалась ';' после поля");
}

void Parser::Type() {
    if (currentTokenCode == TInt) {
        lastType = IntType;
        lastTypeName.clear();
        nextToken();
    } else if (currentTokenCode == TDouble) {
        lastType = DoubleType;
        lastTypeName.clear();
        nextToken();
    } else if (currentTokenCode == TId) {
        lastType = UndefinedType;
        lastTypeName = currentToken;
        nextToken();
    } else {
        error("Ожидался тип (int, double или идентификатор)");
    }
}

void Parser::ConstValueDesc() {
    Type();
    std::string constName = expectId("Ожидался идентификатор константы");

    if (!checkDuplicateId(constName)) {
        throw std::runtime_error("Семантическая ошибка: дублирование константы");
    }

    expect(TEval, "Ожидался '=' для инициализации константы");
    Const();
    expect(TSemicolon, "Ожидалась ';' после константы");

    Node c(constName, ObjConst, lastType);
    Tree* constNode = Tree::SetRight(c);

    if (flagInterpret && constNode && constNode->getNode()) {
        assignValue(constNode->getNode(), lastType, exprValue);
    }
}

void Parser::Const() {
    resetExpr();

    if (currentTokenCode == TConstInt) {
        exprType = IntType;
        if (flagInterpret) {
            exprValue.dataType = TYPE_INT;
            exprValue.dataValue.dataAsInt = std::stoi(currentToken);
        }
        nextToken();
    } else if (currentTokenCode == TConstDouble) {
        exprType = DoubleType;
        if (flagInterpret) {
            exprValue.dataType = TYPE_DOUBLE;
            exprValue.dataValue.dataAsDouble = std::stod(currentToken);
        }
        nextToken();
    } else {
        error("Ожидалась константа");
    }
}

void Parser::CompositeOperator() {
    expect(TLFB, "Ожидалась '{'");
    Tree::semIn();
    OperatorsList();
    Tree::semOut();
    expect(TRFB, "Ожидалась '}'");
}

void Parser::OperatorsList() {
    while (currentTokenCode != TRFB && currentTokenCode != TEnd) {
        if (flagReturn) {
            return;
        }
        Operator();
    }
}

void Parser::Operator() {
    if (currentTokenCode == TSemicolon) {
        nextToken();
        return;
    }
    if (currentTokenCode == TLFB) {
        CompositeOperator();
        return;
    }
    if (currentTokenCode == TWhile) {
        While();
        return;
    }
    if (currentTokenCode == TReturn) {
        Return();
        return;
    }
    if (currentTokenCode == TInt || currentTokenCode == TDouble || currentTokenCode == TId ||
        currentTokenCode == TInc || currentTokenCode == TDec) {
        Statement();
        return;
    }
    error("Ожидался оператор");
}

void Parser::Statement() {
    // prefix ++/-- как отдельный оператор
    if (currentTokenCode == TInc || currentTokenCode == TDec) {
        uint16_t op = currentTokenCode;
        nextToken();

        if (currentTokenCode != TId) {
            error("Ожидался идентификатор после '++/--'");
        }

        bool isMethodCall = false;
        std::string name;
        Tree* targetNode = parseDesignator(false, isMethodCall, name);

        if (!targetNode || !targetNode->getNode()) {
            throw std::runtime_error("Семантическая ошибка");
        }
        if (!checkLValue(targetNode)) {
            std::cerr << "Семантическая ошибка: '++/--' применимы только к переменным/полям" << std::endl;
            throw std::runtime_error("Семантическая ошибка");
        }

        PrimitiveDataType t = targetNode->getNode()->datType;
        if (!(t == IntType || t == DoubleType)) {
            std::cerr << "Семантическая ошибка: '++/--' применимы только к числовым типам" << std::endl;
            throw std::runtime_error("Семантическая ошибка");
        }

        if (flagInterpret) {
            if (t == IntType) {
                int cur = (targetNode->getNode()->data.dataType == TYPE_INT) ? targetNode->getNode()->data.dataValue.dataAsInt : 0;
                cur += (op == TInc) ? 1 : -1;
                TData tmp; tmp.dataType = TYPE_INT; tmp.dataValue.dataAsInt = cur;
                assignValue(targetNode->getNode(), IntType, tmp);
                printAssignment(targetNode->getNode(), name);
            } else {
                double cur = (targetNode->getNode()->data.dataType == TYPE_DOUBLE)
                                 ? targetNode->getNode()->data.dataValue.dataAsDouble
                                 : (targetNode->getNode()->data.dataType == TYPE_INT
                                        ? static_cast<double>(targetNode->getNode()->data.dataValue.dataAsInt)
                                        : 0.0);
                cur += (op == TInc) ? 1.0 : -1.0;
                TData tmp; tmp.dataType = TYPE_DOUBLE; tmp.dataValue.dataAsDouble = cur;
                assignValue(targetNode->getNode(), DoubleType, tmp);
                printAssignment(targetNode->getNode(), name);
            }
        }

        expect(TSemicolon, "Ожидалась ';' после '++/--'");
        return;
    }

    // объявление int/double
    if (currentTokenCode == TInt || currentTokenCode == TDouble) {
        PrimitiveDataType declType = (currentTokenCode == TInt ? IntType : DoubleType);
        nextToken();

        std::string varName = expectId("Ожидался идентификатор переменной");
        if (!checkDuplicateId(varName)) {
            throw std::runtime_error("Семантическая ошибка: дублирование переменной");
        }

        bool hasInit = false;
        if (currentTokenCode == TEval) {
            nextToken();
            Expression();

            if (!compatibleAssign(declType, exprType)) {
                std::cerr << "Семантическая ошибка: несовместимые типы при инициализации '" << varName << "'" << std::endl;
                throw std::runtime_error("Семантическая ошибка");
            }
            hasInit = true;
        }

        Node v(varName, ObjVar, declType, hasInit, "");
        Tree* varNode = Tree::SetRight(v);

        if (flagInterpret && hasInit && varNode && varNode->getNode()) {
            assignValue(varNode->getNode(), declType, exprValue);
            printAssignment(varNode->getNode(), varName);
        }

        expect(TSemicolon, "Ожидалась ';' после объявления");
        return;
    }

    // начинается с идентификатора
    if (currentTokenCode == TId) {
        uint32_t savedPos = scanner->getPos();
        std::string savedToken = currentToken;
        uint16_t savedCode = currentTokenCode;

        // пробуем \"TypeName varName;\"
        nextToken();
        if (currentTokenCode == TId) {
            std::string typeName = savedToken;
            std::string varName  = currentToken;
            nextToken();

            if (!checkDuplicateId(varName)) {
                throw std::runtime_error("Семантическая ошибка: дублирование переменной");
            }

            Tree* classDef = Tree::FindGlobal(typeName);
            if (!classDef || !classDef->getNode() || classDef->getNode()->objType != ObjClass) {
                std::cerr << "Семантическая ошибка: тип '" << typeName << "' не найден как класс" << std::endl;
                throw std::runtime_error("Семантическая ошибка");
            }

            if (currentTokenCode == TEval) {
                std::cerr << "Семантическая ошибка: инициализация объектного типа запрещена" << std::endl;
                throw std::runtime_error("Семантическая ошибка");
            }

            Node v(varName, ObjVar, UndefinedType, false, typeName);
            Tree::SetRight(v);

            expect(TSemicolon, "Ожидалась ';' после объявления");
            return;
        }

        // откат и разбор designator
        scanner->setPos(savedPos);
        currentToken = savedToken;
        currentTokenCode = savedCode;

        bool isMethodCall = false;
        std::string designatorName;
        Tree* targetNode = parseDesignator(true, isMethodCall, designatorName);

        if (!targetNode || !targetNode->getNode()) {
            throw std::runtime_error("Семантическая ошибка");
        }

        if (isMethodCall) {
            if (flagInterpret) {
                (void)execMethod(targetNode, designatorName);
            }
            expect(TSemicolon, "Ожидалась ';' после вызова метода");
            return;
        }

        // postfix ++/-- как оператор
        if (currentTokenCode == TInc || currentTokenCode == TDec) {
            if (!checkLValue(targetNode)) {
                std::cerr << "Семантическая ошибка: '++/--' применимы только к переменным/полям" << std::endl;
                throw std::runtime_error("Семантическая ошибка");
            }

            PrimitiveDataType t = targetNode->getNode()->datType;
            if (!(t == IntType || t == DoubleType)) {
                std::cerr << "Семантическая ошибка: '++/--' применимы только к числовым типам" << std::endl;
                throw std::runtime_error("Семантическая ошибка");
            }

            uint16_t op = currentTokenCode;
            nextToken();

            if (flagInterpret) {
                if (t == IntType) {
                    int cur = (targetNode->getNode()->data.dataType == TYPE_INT) ? targetNode->getNode()->data.dataValue.dataAsInt : 0;
                    cur += (op == TInc) ? 1 : -1;
                    TData tmp; tmp.dataType = TYPE_INT; tmp.dataValue.dataAsInt = cur;
                    assignValue(targetNode->getNode(), IntType, tmp);
                    printAssignment(targetNode->getNode(), designatorName);
                } else {
                    double cur = (targetNode->getNode()->data.dataType == TYPE_DOUBLE)
                                     ? targetNode->getNode()->data.dataValue.dataAsDouble
                                     : (targetNode->getNode()->data.dataType == TYPE_INT
                                            ? static_cast<double>(targetNode->getNode()->data.dataValue.dataAsInt)
                                            : 0.0);
                    cur += (op == TInc) ? 1.0 : -1.0;
                    TData tmp; tmp.dataType = TYPE_DOUBLE; tmp.dataValue.dataAsDouble = cur;
                    assignValue(targetNode->getNode(), DoubleType, tmp);
                    printAssignment(targetNode->getNode(), designatorName);
                }
            }

            expect(TSemicolon, "Ожидалась ';' после '++/--'");
            return;
        }

        if (currentTokenCode == TEval ||
            currentTokenCode == TPlusEq || currentTokenCode == TMinusEq ||
            currentTokenCode == TMultEq || currentTokenCode == TDivEq  || currentTokenCode == TModEq) {

            if (!checkLValue(targetNode)) {
                std::cerr << "Семантическая ошибка: слева от присваивания должно быть изменяемое значение" << std::endl;
                throw std::runtime_error("Семантическая ошибка");
            }

            PrimitiveDataType leftType = targetNode->getNode()->datType;
            if (!(leftType == IntType || leftType == DoubleType)) {
                std::cerr << "Семантическая ошибка: присваивание возможно только для числовых типов" << std::endl;
                throw std::runtime_error("Семантическая ошибка");
            }

            uint16_t assignOp = currentTokenCode;
            nextToken();
            Expression();

            if (!(exprType == IntType || exprType == DoubleType)) {
                std::cerr << "Семантическая ошибка: справа должно быть числовое выражение" << std::endl;
                throw std::runtime_error("Семантическая ошибка");
            }

            if (assignOp == TEval) {
                if (!compatibleAssign(leftType, exprType)) {
                    std::cerr << "Семантическая ошибка: несовместимые типы при присваивании" << std::endl;
                    throw std::runtime_error("Семантическая ошибка");
                }

                if (flagInterpret) {
                    assignValue(targetNode->getNode(), leftType, exprValue);
                    printAssignment(targetNode->getNode(), designatorName);
                }

                expect(TSemicolon, "Ожидалась ';' после присваивания");
                return;
            }

            uint16_t opToken = 0;
            switch (assignOp) {
                case TPlusEq:  opToken = TPlus;  break;
                case TMinusEq: opToken = TMinus; break;
                case TMultEq:  opToken = TMult;  break;
                case TDivEq:   opToken = TDiv;   break;
                case TModEq:   opToken = TMod;   break;
                default: opToken = 0; break;
            }

            PrimitiveDataType resType;
            TData leftVal;

            if (leftType == DoubleType) {
                leftVal.dataType = TYPE_DOUBLE;
                if (targetNode->getNode()->data.dataType == TYPE_DOUBLE) leftVal.dataValue.dataAsDouble = targetNode->getNode()->data.dataValue.dataAsDouble;
                else if (targetNode->getNode()->data.dataType == TYPE_INT) leftVal.dataValue.dataAsDouble = static_cast<double>(targetNode->getNode()->data.dataValue.dataAsInt);
                else leftVal.dataValue.dataAsDouble = 0.0;
            } else {
                leftVal.dataType = TYPE_INT;
                if (targetNode->getNode()->data.dataType == TYPE_INT) leftVal.dataValue.dataAsInt = targetNode->getNode()->data.dataValue.dataAsInt;
                else if (targetNode->getNode()->data.dataType == TYPE_DOUBLE) leftVal.dataValue.dataAsInt = static_cast<int>(targetNode->getNode()->data.dataValue.dataAsDouble);
                else leftVal.dataValue.dataAsInt = 0;
            }

            TData res = evalBinary(opToken, leftVal, leftType, exprValue, exprType, resType);

            if (!compatibleAssign(leftType, resType)) {
                std::cerr << "Семантическая ошибка: несовместимые типы при составном присваивании" << std::endl;
                throw std::runtime_error("Семантическая ошибка");
            }

            if (flagInterpret) {
                assignValue(targetNode->getNode(), leftType, res);
                printAssignment(targetNode->getNode(), designatorName);
            }

            expect(TSemicolon, "Ожидалась ';' после присваивания");
            return;
        }

        error("Неподдерживаемая конструкция после идентификатора");
    }

    error("Ожидался тип или идентификатор");
}

void Parser::Assign() {
    error("Assign() не используется в данной реализации");
}

void Parser::While() {
    expect(TWhile, "Ожидалось 'while'");
    uint32_t ukCond = scanner->getTokenStartPos();

    const bool outerInterpret = flagInterpret;

    debugEvent("Начинаю while");
    debugFlag("while(entry)");

    while (true) {
        setUK(ukCond);
        debugEvent("while: перемотка на условие");
        debugFlag("while(rewind)");

        expect(TLB, "Ожидалась '(' после while");
        Expression();

        if (!(exprType == IntType || exprType == DoubleType)) {
            std::cerr
                << "Семантическая ошибка: условие while должно быть числовым"
                << std::endl;
            throw std::runtime_error("Семантическая ошибка");
        }

        expect(TRB, "Ожидалась ')' после условия");

        bool cond = false;
        if (outerInterpret) {
            cond = condToBool(exprValue);
        }

        debugEvent(std::string("while: условие=") + (cond ? "true" : "false"));

        flagInterpret = (outerInterpret && cond);
        debugEvent("while: устанавливаю flagInterpret = outer && cond");
        debugFlag("while(body-flag)");

        Operator();

        if (flagReturn) {
            debugEvent("while: выход по return (flagReturn=TRUE)");
            debugFlag("while(return)");
            return;
        }

        flagInterpret = outerInterpret;
        debugEvent("while: восстанавливаю flagInterpret во внешний режим");
        debugFlag("while(restore)");

        if (!outerInterpret) {
            debugEvent("while: внешний контекст DOWN -> выхожу из while");
            debugFlag("while(exit-outer-down)");
            return;
        }

        if (!cond) {
            debugEvent("while: условие ложно -> выхожу из while");
            debugFlag("while(exit-cond-false)");
            return;
        }

        debugEvent("while: следующая итерация");
    }
}


TData Parser::execMethod(Tree* methodNode, const std::string& fullName) {
    // (оставлено как в твоём файле; эта часть не влияет на ошибку while)
    TData res;
    res.dataType = TYPE_UNKNOWN;

    auto it = methodBodyPos.find(methodNode);
    if (it == methodBodyPos.end()) {
        throw std::runtime_error("Semantic error: method start not found");
    }

    uint32_t savedPos = getUK();
    std::string savedTok = currentToken;
    uint16_t savedCode = currentTokenCode;
    Tree* savedCur = Tree::getCurrent();

    bool savedInterpret = flagInterpret;
    bool savedReturn = flagReturn;
    PrimitiveDataType savedRetType = returnType;
    TData savedRetVal = returnValue;

    flagInterpret = true;

    debugEvent(std::string("Перехожу к методу: ") + fullName);
    debugFlag("method(enter)");

    flagReturn = false;
    returnType = methodNode->getNode()->datType;
    returnValue = TData();

    Tree::setCurrent(methodNode);
    setUK(it->second);

    Tree::semIn();
    OperatorsList();
    Tree::semOut();
    expect(TRFB, "Ожидалась '}'");

    res = returnValue;

    debugEvent(std::string("Возвращаю значение из метода: ") + fullName);
    debugValue("[DEBUG] return value", res, returnType);

    flagInterpret = savedInterpret;
    flagReturn = savedReturn;
    returnType = savedRetType;
    returnValue = savedRetVal;

    Tree::setCurrent(savedCur);
    scanner->setPos(savedPos);
    currentToken = savedTok;
    currentTokenCode = savedCode;

    debugFlag("method(exit)");
    return res;
}

void Parser::Return() {
    debugEvent("Встречен оператор return");
    debugFlag("return(seen)");

    expect(TReturn, "Ожидалось 'return'");
    Expression();

    if (!(exprType == IntType || exprType == DoubleType)) {
        std::cerr
            << "Семантическая ошибка: оператор return должен возвращать числовое выражение"
            << std::endl;
        throw std::runtime_error("Семантическая ошибка");
    }

    expect(TSemicolon, "Ожидалась ';' после return");

    if (flagInterpret) {
        returnValue = exprValue;

        flagReturn = true;
        debugEvent("return: поднимаю flagReturn=TRUE");
        debugFlag("return(raise)");

        flagInterpret = false;
        debugEvent("return: опускаю flagInterpret=DOWN");
        debugFlag("return(after)");
    } else {
        debugEvent("return: пропущен (flagInterpret DOWN)");
        debugFlag("return(skip)");
    }
}

void Parser::Expression() {
    resetExpr();

    Sum();
    TData leftVal = exprValue;
    PrimitiveDataType leftType = exprType;

    if (currentTokenCode == TL || currentTokenCode == TG ||
        currentTokenCode == TLE || currentTokenCode == TGE ||
        currentTokenCode == TNotEq || currentTokenCode == TEq) {

        uint16_t op = currentTokenCode;
        nextToken();

        Sum();
        TData rightVal = exprValue;
        PrimitiveDataType rightType = exprType;

        PrimitiveDataType resType;
        TData res = evalBinary(op, leftVal, leftType, rightVal, rightType, resType);
        exprValue = res;
        exprType = resType;
    }

    debugValue("[DEBUG] Результат выражения", exprValue, exprType);
}

void Parser::Sum() {
    Mult();
    TData accVal = exprValue;
    PrimitiveDataType accType = exprType;

    while (currentTokenCode == TPlus || currentTokenCode == TMinus) {
        uint16_t op = currentTokenCode;
        nextToken();

        Mult();
        TData rightVal = exprValue;
        PrimitiveDataType rightType = exprType;

        PrimitiveDataType resType;
        TData res = evalBinary(op, accVal, accType, rightVal, rightType, resType);
        accVal = res;
        accType = resType;

        debugValue("[DEBUG] Шаг", accVal, accType);
    }

    exprValue = accVal;
    exprType = accType;
}

void Parser::Mult() {
    Unary();
    TData accVal = exprValue;
    PrimitiveDataType accType = exprType;

    while (currentTokenCode == TMult ||
           currentTokenCode == TDiv ||
           currentTokenCode == TMod) {

        uint16_t op = currentTokenCode;
        nextToken();

        Unary();
        TData rightVal = exprValue;
        PrimitiveDataType rightType = exprType;

        PrimitiveDataType resType;
        TData res = evalBinary(op, accVal, accType, rightVal, rightType, resType);
        accVal = res;
        accType = resType;

        debugValue("[DEBUG] Шаг", accVal, accType);
    }

    exprValue = accVal;
    exprType = accType;
}

void Parser::Unary() {
    // prefix ++/-- в выражениях
    if (currentTokenCode == TInc || currentTokenCode == TDec) {
        uint16_t op = currentTokenCode;
        nextToken();

        if (currentTokenCode != TId) {
            error("Ожидался идентификатор после '++/--'");
        }

        bool isMethodCall = false;
        std::string fullName;
        Tree* node = parseDesignator(false, isMethodCall, fullName);

        if (!node || !node->getNode()) {
            throw std::runtime_error("Семантическая ошибка");
        }

        if (!checkLValue(node)) {
            std::cerr << "Семантическая ошибка: '++/--' применимы только к переменным/полям" << std::endl;
            throw std::runtime_error("Семантическая ошибка");
        }

        exprType = node->getNode()->datType;
        if (!(exprType == IntType || exprType == DoubleType)) {
            std::cerr << "Семантическая ошибка: '++/--' применимы только к числовым типам" << std::endl;
            throw std::runtime_error("Семантическая ошибка");
        }

        if (flagInterpret) {
            if (exprType == IntType) {
                int cur = (node->getNode()->data.dataType == TYPE_INT) ? node->getNode()->data.dataValue.dataAsInt : 0;
                cur += (op == TInc) ? 1 : -1;
                TData tmp; tmp.dataType = TYPE_INT; tmp.dataValue.dataAsInt = cur;
                assignValue(node->getNode(), IntType, tmp);
                exprValue = tmp;
            } else {
                double cur = (node->getNode()->data.dataType == TYPE_DOUBLE)
                                 ? node->getNode()->data.dataValue.dataAsDouble
                                 : (node->getNode()->data.dataType == TYPE_INT
                                        ? static_cast<double>(node->getNode()->data.dataValue.dataAsInt)
                                        : 0.0);
                cur += (op == TInc) ? 1.0 : -1.0;
                TData tmp; tmp.dataType = TYPE_DOUBLE; tmp.dataValue.dataAsDouble = cur;
                assignValue(node->getNode(), DoubleType, tmp);
                exprValue = tmp;
            }
        } else {
            exprValue.dataType = TYPE_UNKNOWN;
        }

        return;
    }

    bool negate = false;

    if (currentTokenCode == TPlus || currentTokenCode == TMinus) {
        if (currentTokenCode == TMinus) {
            negate = true;
        }
        nextToken();
    }

    BaseExp();

    if (negate && flagInterpret) {
        if (exprValue.dataType == TYPE_INT) {
            exprValue.dataValue.dataAsInt = -exprValue.dataValue.dataAsInt;
        } else if (exprValue.dataType == TYPE_DOUBLE) {
            exprValue.dataValue.dataAsDouble = -exprValue.dataValue.dataAsDouble;
        }
        debugValue("[DEBUG] унарный минус", exprValue, exprType);
    }
}

Tree* Parser::parseDesignator(bool allowMethodCall,
                              bool& isMethodCall,
                              std::string& fullName) {
    if (currentTokenCode != TId) {
        error("Ожидался идентификатор");
    }

    fullName = currentToken;
    std::string name = currentToken;

    if (!checkId(name)) {
        throw std::runtime_error(
            "Семантическая ошибка: использование необъявленного идентификатора");
    }

    Tree* node = Tree::getCurrent()->FindUp(name);
    if (!node) {
        throw std::runtime_error(
            "Семантическая ошибка: внутренний поиск идентификатора провалился");
    }

    nextToken();

    while (currentTokenCode == TPoint) {
        nextToken();
        std::string member = expectId("Ожидался идентификатор после '.'");
        fullName += ".";
        fullName += member;

        Node* curNode = node->getNode();
        if (!curNode) {
            throw std::runtime_error("Семантическая ошибка");
        }

        Tree* classNode = nullptr;
        if ((curNode->objType == ObjVar || curNode->objType == ObjField) &&
            !curNode->typeName.empty()) {
            classNode = Tree::FindGlobal(curNode->typeName);
        } else {
            std::cerr
                << "Семантическая ошибка: доступ к члену у не объектного типа"
                << std::endl;
            throw std::runtime_error("Семантическая ошибка");
        }

        if (!classNode || !classNode->getNode() ||
            classNode->getNode()->objType != ObjClass) {
            std::cerr << "Семантическая ошибка: тип '"
                      << (curNode->typeName.empty() ? curNode->id : curNode->typeName)
                      << "' не найден как класс" << std::endl;
            throw std::runtime_error("Семантическая ошибка");
        }

        Tree* memberNode = classNode->FindDownLeft(member);
        if (!memberNode || !memberNode->getNode()) {
            std::cerr << "Семантическая ошибка: член '" << member
                      << "' отсутствует в классе" << std::endl;
            throw std::runtime_error("Семантическая ошибка");
        }

        node = memberNode;
    }

    isMethodCall = false;

    if (currentTokenCode == TLB) {
        if (!allowMethodCall) {
            error("Вызов метода в недопустимом контексте");
        }

        if (!node || !node->getNode() ||
            node->getNode()->objType != ObjMethod) {
            std::cerr
                << "Семантическая ошибка: попытка вызвать не-метод"
                << std::endl;
            throw std::runtime_error("Семантическая ошибка");
        }

        expect(TLB, "Ожидалась '(' при вызове метода");
        expect(TRB, "Ожидалась ')' при вызове метода");
        isMethodCall = true;
    }

    return node;
}

void Parser::BaseExp() {
    if (currentTokenCode == TId) {
        bool isMethodCall = false;
        std::string fullName;
        Tree* node = parseDesignator(true, isMethodCall, fullName);

        if (!node || !node->getNode()) {
            throw std::runtime_error("Семантическая ошибка");
        }

        if (isMethodCall) {
            exprType = node->getNode()->datType;
            if (flagInterpret) {
                TData ret = execMethod(node, fullName);
                exprValue = ret;
                if (exprType == IntType) exprValue.dataType = TYPE_INT;
                else if (exprType == DoubleType) exprValue.dataType = TYPE_DOUBLE;
            } else {
                exprValue.dataType = TYPE_UNKNOWN;
            }
            return;
        }

        exprType = node->getNode()->datType;
        if (flagInterpret) {
            if (node->getNode()->data.dataType == TYPE_INT) {
                exprValue.dataType = TYPE_INT;
                exprValue.dataValue.dataAsInt =
                    node->getNode()->data.dataValue.dataAsInt;
            } else if (node->getNode()->data.dataType == TYPE_DOUBLE) {
                exprValue.dataType = TYPE_DOUBLE;
                exprValue.dataValue.dataAsDouble =
                    node->getNode()->data.dataValue.dataAsDouble;
            } else {
                exprValue.dataType = TYPE_UNKNOWN;
            }
        } else {
            exprValue.dataType = TYPE_UNKNOWN;
        }

        // postfix ++/-- в выражениях
        if (currentTokenCode == TInc || currentTokenCode == TDec) {
            uint16_t op = currentTokenCode;
            nextToken();

            if (!checkLValue(node)) {
                std::cerr << "Семантическая ошибка: '++/--' применимы только к переменным/полям" << std::endl;
                throw std::runtime_error("Семантическая ошибка");
            }
            if (!(exprType == IntType || exprType == DoubleType)) {
                std::cerr << "Семантическая ошибка: '++/--' применимы только к числовым типам" << std::endl;
                throw std::runtime_error("Семантическая ошибка");
            }

            if (flagInterpret) {
                if (exprType == IntType) {
                    int oldv = (exprValue.dataType == TYPE_INT) ? exprValue.dataValue.dataAsInt : 0;
                    int newv = oldv + ((op == TInc) ? 1 : -1);
                    TData tmp; tmp.dataType = TYPE_INT; tmp.dataValue.dataAsInt = newv;
                    assignValue(node->getNode(), IntType, tmp);
                    exprValue.dataType = TYPE_INT;
                    exprValue.dataValue.dataAsInt = oldv;
                } else {
                    double oldv = (exprValue.dataType == TYPE_DOUBLE) ? exprValue.dataValue.dataAsDouble
                                  : (exprValue.dataType == TYPE_INT ? static_cast<double>(exprValue.dataValue.dataAsInt) : 0.0);
                    double newv = oldv + ((op == TInc) ? 1.0 : -1.0);
                    TData tmp; tmp.dataType = TYPE_DOUBLE; tmp.dataValue.dataAsDouble = newv;
                    assignValue(node->getNode(), DoubleType, tmp);
                    exprValue.dataType = TYPE_DOUBLE;
                    exprValue.dataValue.dataAsDouble = oldv;
                }
            }
        }

        return;
    } else if (currentTokenCode == TConstInt) {
        exprType = IntType;
        if (flagInterpret) {
            exprValue.dataType = TYPE_INT;
            exprValue.dataValue.dataAsInt = std::stoi(currentToken);
        } else {
            exprValue.dataType = TYPE_UNKNOWN;
        }
        nextToken();
        return;
    } else if (currentTokenCode == TConstDouble) {
        exprType = DoubleType;
        if (flagInterpret) {
            exprValue.dataType = TYPE_DOUBLE;
            exprValue.dataValue.dataAsDouble = std::stod(currentToken);
        } else {
            exprValue.dataType = TYPE_UNKNOWN;
        }
        nextToken();
        return;
    } else if (currentTokenCode == TLB) {
        nextToken();
        Expression();
        expect(TRB, "Ожидалась ')'");
        return;
    }

    error("Ожидалось выражение");
}

void Parser::MethodCall() {
    std::string obj = expectId("Ожидался идентификатор объекта");
    expect(TPoint, "Ожидалась '.'");
    std::string method = expectId("Ожидался идентификатор метода");
    expect(TLB, "Ожидалась '('");
    expect(TRB, "Ожидалась ')'");
}
