#include <iostream>
#include "Scanner.hpp"

int main()
{
    Scanner* scanner = new Scanner("C:\\vs code\\c++\\trans\\test.cpp");

    std::string token{};
    uint16_t tokenCode{};

    do
    {
        tokenCode = scanner->scan(token);
        std::cout << "Token: " << token << "\t[code: " << tokenCode << "]" << std::endl;
    } while (tokenCode != TEnd && tokenCode != Terr);
}