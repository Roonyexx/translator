#pragma once

#include <memory>
#include <string>

enum TypeObject {
    ObjEmpty = 0,
    ObjVar,
    ObjConst,
    ObjClass,
    ObjMethod,
    ObjFunc,
    ObjField
};

enum PrimitiveDataType {
    UndefinedType = 0,
    IntType,
    DoubleType
};

enum DataType {
    TYPE_UNKNOWN = 0,
    TYPE_INT,
    TYPE_DOUBLE
};

union DataValue {
    int dataAsInt;
    double dataAsDouble;

    DataValue() : dataAsInt(0) {}
};

struct TData {
    DataType dataType;
    DataValue dataValue;

    TData() : dataType(TYPE_UNKNOWN), dataValue() {}
};

struct Node {
    std::string id;
    TypeObject objType;
    PrimitiveDataType datType;
    bool isInitialized;
    std::string typeName;
    TData data;

    Node()
        : id(),
          objType(ObjEmpty),
          datType(UndefinedType),
          isInitialized(false),
          typeName(),
          data() {}

    Node(const std::string& i,
         TypeObject o = ObjEmpty,
         PrimitiveDataType d = UndefinedType,
         bool init = false,
         const std::string& tn = std::string())
        : id(i),
          objType(o),
          datType(d),
          isInitialized(init),
          typeName(tn),
          data() {
        switch (datType) {
        case IntType:
            data.dataType = TYPE_INT;
            data.dataValue.dataAsInt = 0;
            break;
        case DoubleType:
            data.dataType = TYPE_DOUBLE;
            data.dataValue.dataAsDouble = 0.0;
            break;
        default:
            data.dataType = TYPE_UNKNOWN;
            break;
        }
    }
};

class Tree {
public:
    Tree(Node* data = nullptr, Tree* parent = nullptr);
    ~Tree();

    static void setCurrent(Tree* cur);
    static Tree* getCurrent();

    static Tree* SetRight(const Node& data);
    static Tree* SetLeft(const Node& data);

    Tree* FindUp(const std::string& id);
    Tree* FindUpOneLevel(const std::string& id);
    Tree* FindDownLeft(const std::string& id);

    static Tree* FindGlobal(const std::string& id);

    static void PrintTree(Tree* from = nullptr);

    static void semIn();
    static void semOut();

    static std::string TypeName(PrimitiveDataType t);
    static std::string ObjName(TypeObject o);

    Node* getNode() { return node.get(); }
    Tree* getParent() { return parent; }
    Tree* getLeft() { return firstChild; }
    Tree* getRight() { return nextSibling; }

private:
    std::unique_ptr<Node> node;
    Tree* parent;
    Tree* firstChild;
    Tree* nextSibling;

    static Tree* root;
    static Tree* current;

    static void printRec(Tree* t, int indent);
};

bool checkId(const std::string& id);
bool checkDuplicateId(const std::string& id);
bool checkLValue(Tree* node);
bool checkAssignTypes(Tree* left, Tree* right);
bool checkArithmeticTypes(Tree* op1, Tree* op2);
bool checkCompareTypes(Tree* op1, Tree* op2);
bool checkCondition(Tree* expr);
PrimitiveDataType getExprType(Tree* op1, Tree* op2);
Tree* checkClassMember(Tree* classNode, const std::string& member);
Tree* checkMethod(Tree* classNode, const std::string& method);
bool checkMethodReturn(Tree* methodNode);
