#include <iostream>
#include "Scanner.hpp"
#include "Parser.hpp"

int main()
{
    try
    {
        Scanner* scanner = new Scanner("C:\\vs code\\c++\\trans\\test.cpp");
        Parser* parser = new Parser(scanner);
        
        parser->parse();
        
        delete parser;
        delete scanner;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
