#include "Tree.hpp"

Tree* Tree::root    = nullptr;
Tree* Tree::current = nullptr;

Tree::Tree(Node* data, Tree* parent)
    : node(data ? std::unique_ptr<Node>(new Node(*data)) : nullptr),
      parent(parent),
      firstChild(nullptr),
      nextSibling(nullptr)
{
}

Tree::~Tree()
{
    delete firstChild;
    delete nextSibling;
}

void Tree::setCurrent(Tree* cur)
{
    current = cur;
}

Tree* Tree::getCurrent()
{
    return current;
}

Tree* Tree::SetRight(const Node& data)
{
    // Добавление символа в текущую область (current)
    if (!root)
    {
        Tree* n = new Tree(new Node(data), nullptr);
        root    = n;
        current = n;
        return n;
    }

    if (!current)
        current = root;

    // добавляем как ребёнка current
    if (!current->firstChild)
    {
        Tree* child   = new Tree(new Node(data), current);
        current->firstChild = child;
        return child;
    }
    else
    {
        Tree* it = current->firstChild;
        while (it->nextSibling)
            it = it->nextSibling;
        Tree* child = new Tree(new Node(data), current);
        it->nextSibling = child;
        return child;
    }
}

Tree* Tree::SetLeft(const Node& data)
{
    // Добавление узла-области и переход в неё
    Tree* n = SetRight(data);
    current = n;
    return n;
}

Tree* Tree::FindUp(const std::string& id)
{
    Tree* scope = this;
    while (scope)
    {
        Tree* child = scope->firstChild;
        while (child)
        {
            if (child->node && child->node->id == id)
                return child;
            child = child->nextSibling;
        }
        scope = scope->parent;
    }
    return nullptr;
}

Tree* Tree::FindUpOneLevel(const std::string& id)
{
    Tree* scope = this;
    Tree* child = scope->firstChild;
    while (child)
    {
        if (child->node && child->node->id == id)
            return child;
        child = child->nextSibling;
    }
    return nullptr;
}

Tree* Tree::FindDownLeft(const std::string& id)
{
    Tree* child = firstChild;
    while (child)
    {
        if (child->node && child->node->id == id)
            return child;
        child = child->nextSibling;
    }
    return nullptr;
}

// рекурсивный поиск класса по всему дереву
static Tree* findClassRecursive(Tree* node, const std::string& id)
{
    if (!node) return nullptr;

    if (node->getNode() &&
        node->getNode()->id == id &&
        node->getNode()->objType == ObjClass)
        return node;

    Tree* res = findClassRecursive(node->getLeft(), id);
    if (res) return res;

    return findClassRecursive(node->getRight(), id);
}

Tree* Tree::FindGlobal(const std::string& id)
{
    if (!root) return nullptr;
    return findClassRecursive(root, id);
}

void Tree::printRec(Tree* t, int indent) {
    if (!t) return;
    std::string pad(indent, ' ');

    // если это узел-секция [Scope] и у него нет "собственного" имени -> печатаем только детей
    if (t->node && t->node->id == "[Scope]") {
        Tree* it = t->firstChild;
        while (it) {
            printRec(it, indent);    // не увеличиваем отступ для тесной вложенности,
            it = it->nextSibling;
        }
        return;
    }

    if (t->node) {
        std::cout << pad << t->node->id << " ";
        if (t->node->datType != UndefinedType)
            std::cout << "(" << TypeName(t->node->datType) << ")";
        else if (!t->node->typeName.empty())
            std::cout << "(" << t->node->typeName << ")";
        else
            std::cout << "(undefined)";
        if (t->node->objType != ObjEmpty) std::cout << " [" << ObjName(t->node->objType) << "]";
        if (t->node->isInitialized) std::cout << " {inited}";
        std::cout << std::endl;
    }

    if (t->firstChild) {
        Tree* it = t->firstChild;
        while (it) {
            printRec(it, indent + 4);
            it = it->nextSibling;
        }
    }
}

void Tree::PrintTree(Tree* from)
{
    std::cout << "\n--- Семантическое дерево ---\n";
    (void)from;

    if (!root)
    {
        std::cout << "(пусто)\n---------------------------\n";
        return;
    }

    printRec(root, 0);
    std::cout << "---------------------------\n";
}

std::string Tree::TypeName(PrimitiveDataType t)
{
    switch (t)
    {
    case IntType:    return "int";
    case DoubleType: return "double";
    default:         return "undefined";
    }
}

std::string Tree::ObjName(TypeObject o)
{
    switch (o)
    {
    case ObjVar:   return "var";
    case ObjConst: return "const";
    case ObjClass: return "class";
    case ObjMethod:return "method";
    case ObjFunc:  return "func";
    default:       return "obj";
    }
}

void Tree::semIn()
{
    Node n("[Scope]", ObjEmpty, UndefinedType, false, "");
    SetLeft(n);
}

void Tree::semOut()
{
    if (!current) return;
    if (current->parent)
        current = current->parent;
}

// ===== семантические функции =====

bool checkId(const std::string& id)
{
    Tree* cur = Tree::getCurrent();
    if (!cur)
    {
        std::cerr << "Семантическая ошибка: дерево не инициализировано\n";
        return false;
    }
    Tree* found = cur->FindUp(id);
    if (!found)
    {
        std::cerr << "Семантическая ошибка: идентификатор '" << id
                  << "' не объявлен (использование до объявления)\n";
        return false;
    }
    return true;
}

bool checkDuplicateId(const std::string& id)
{
    Tree* cur = Tree::getCurrent();
    if (!cur)
    {
        std::cerr << "Семантическая ошибка: дерево не инициализировано\n";
        return false;
    }
    Tree* found = cur->FindUpOneLevel(id);
    if (found)
    {
        std::cerr << "Семантическая ошибка: дублирующее объявление '"
                  << id << "' в одной области\n";
        return false;
    }
    return true;
}

bool checkLValue(Tree* node)
{
    if (!node || !node->getNode()) return false;
    return node->getNode()->objType == ObjVar;
}

bool checkAssignTypes(Tree* left, Tree* right)
{
    if (!left || !right) return false;
    PrimitiveDataType l = left->getNode()->datType;
    PrimitiveDataType r = right->getNode()->datType;

    if (l == UndefinedType || r == UndefinedType)
    {
        std::cerr << "Семантическая ошибка: неопределённый тип в присваивании\n";
        return false;
    }

    if (l == r) return true;
    if (l == DoubleType && r == IntType) return true;

    std::cerr << "Семантическая ошибка: несовместимые типы в присваивании ("
              << Tree::TypeName(l) << " := " << Tree::TypeName(r) << ")\n";
    return false;
}

bool checkArithmeticTypes(Tree* op1, Tree* op2)
{
    if (!op1 || !op2) return false;
    PrimitiveDataType a = op1->getNode()->datType;
    PrimitiveDataType b = op2->getNode()->datType;

    if ((a == IntType || a == DoubleType) &&
        (b == IntType || b == DoubleType))
        return true;

    std::cerr << "Семантическая ошибка: арифметическая операция над нечисловыми типами\n";
    return false;
}

bool checkCompareTypes(Tree* op1, Tree* op2)
{
    return checkArithmeticTypes(op1, op2);
}

bool checkCondition(Tree* expr)
{
    if (!expr || !expr->getNode()) return false;
    PrimitiveDataType t = expr->getNode()->datType;
    return (t == IntType || t == DoubleType);
}

Tree* checkClassMember(Tree* classNode, const std::string& member)
{
    if (!classNode) return nullptr;
    return classNode->FindDownLeft(member);
}

Tree* checkMethod(Tree* classNode, const std::string& method)
{
    return checkClassMember(classNode, method);
}

bool checkMethodReturn(Tree* methodNode)
{
    if (!methodNode || !methodNode->getNode()) return false;
    return methodNode->getNode()->datType != UndefinedType;
}

PrimitiveDataType getExprType(Tree* op1, Tree* op2)
{
    if (!op1 || !op2) return UndefinedType;
    PrimitiveDataType a = op1->getNode()->datType;
    PrimitiveDataType b = op2->getNode()->datType;
    if (a == DoubleType || b == DoubleType) return DoubleType;
    if (a == IntType    || b == IntType)    return IntType;
    return UndefinedType;
}
