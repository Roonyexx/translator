#pragma once

#include <string>
#include <memory>
#include <iostream>

enum TypeObject {
    ObjEmpty = 0,
    ObjVar,
    ObjConst,
    ObjClass,
    ObjMethod,
    ObjFunc
};

enum PrimitiveDataType {
    UndefinedType = 0,
    IntType,
    DoubleType
};

struct Node {
    std::string id;             
    TypeObject objType;        
    PrimitiveDataType datType; 
    bool isInitialized;         
    std::string typeName;

    Node()
        : id(), objType(ObjEmpty), datType(UndefinedType),
          isInitialized(false), typeName()
    {}

    Node(const std::string& i,
         TypeObject o = ObjEmpty,
         PrimitiveDataType d = UndefinedType,
         bool init = false,
         const std::string& tn = std::string())
        : id(i), objType(o), datType(d),
          isInitialized(init), typeName(tn)
    {}
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

    Node* getNode()        { return node.get(); }
    Tree* getParent()      { return parent; }
    Tree* getLeft()        { return firstChild; }
    Tree* getRight()       { return nextSibling; }

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
Tree*  checkClassMember(Tree* classNode, const std::string& member);
Tree*  checkMethod(Tree* classNode, const std::string& method);
bool   checkMethodReturn(Tree* methodNode);
