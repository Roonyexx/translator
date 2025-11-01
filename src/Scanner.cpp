#include "Scanner.hpp"

#include <fstream>
#include <iostream>

Scanner::Scanner(const std::string& filename) : pos{ 0 }
{
    getTextFromFile(filename);
}

uint16_t Scanner::scan(std::string &token)
{
    token = "";

    while (programText[pos] == ' '  || 
           programText[pos] == '\t' || 
           programText[pos] == '\n')
        pos++;

    if(programText[pos] == '\0')
    {
        token = "End of module";
        return TEnd;
    }

    if(isDigit(programText[pos]))
    {
        while(isDigit(programText[pos]))
            token += programText[pos++];
        if(programText[pos] != '.') return TConstInt;
        else
        {
            token += programText[pos++];
            if(!isDigit(programText[pos]))
            {
                printError("invalid constant", token);
                return Terr;
            }

            while(isDigit(programText[pos]))
                token += programText[pos++];

            return TConstDouble;
        }
    }

    else if(isLetter(programText[pos]))
    {
        while(isLetter(programText[pos]) || isDigit(programText[pos]))
            token += programText[pos++];

        if(isKeyword(token))
            return keywords.at(token);

        return TId;
    }

    else if(programText[pos] == '.')
	{
		token += programText[pos++];
		return TPoint;
	}

    else if(programText[pos] == ',')
    {
        token += programText[pos++];
        return TComma;
    }

    else if(programText[pos] == ';')
    {
        token += programText[pos++];
        return TSemicolon;
    }

    else if(programText[pos] == '(')
    {
        token += programText[pos++];
        return TLB;
    }

    else if(programText[pos] == ')')
    {
        token += programText[pos++];
        return TRB;
    }

    else if(programText[pos] == '{')
    {
        token += programText[pos++];
        return TLFB;
    }

    else if(programText[pos] == '}')
    {
        token += programText[pos++];
        return TRFB;
    }

    else if(programText[pos] == '=')
    {
        token += programText[pos++];
        if(programText[pos] == '=')
        {
            token += programText[pos++];
            return TEq;
        }
        return TEval;
    }

    else if(programText[pos] == '+')
    {
        token += programText[pos++];
        if(programText[pos] == '=')
        {
            token += programText[pos++];
            return TPlusEq;
        }
        
        if(programText[pos] == '+')
        {
            token += programText[pos++];
            return TInc;
        }
        return TPlus;
    }

    else if(programText[pos] == '-')
    {
        token += programText[pos++];
        if(programText[pos] == '=')
        {
            token += programText[pos++];
            return TMinusEq;
        }
        
        if(programText[pos] == '-')
        {
            token += programText[pos++];
            return TDec;
        }
        return TMinus;
    }

	else if(programText[pos] == '/')
	{
		token = programText[pos++];

		if (programText[pos] == '=')
		{
			token += programText[pos++];
			return TDivEq;
		}

		return TDiv;
	} 

    else if(programText[pos] == '*')
	{
		token = programText[pos++];

		if (programText[pos] == '=')
		{
			token += programText[pos++];
			return TMultEq;
		}

		return TMult;
	}

    else if(programText[pos] == '%')
	{
		token = programText[pos++];

		if (programText[pos] == '=')
		{
			token += programText[pos++];
			return TModEq;
		}

		return TMod;
	}

    else if(programText[pos] == '>')
    {
        token += programText[pos++];

        if(programText[pos] == '=')
        {
            token += programText[pos++];
            return TGE;
        }

        return TL;
    }

    else if(programText[pos] == '<')
    {
        token += programText[pos++];

        if(programText[pos] == '=')
        {
            token += programText[pos++];
            return TLE;
        }
        
        return TL;
    }

    else if(programText[pos] == '!')
    {
        token += programText[pos++];

        if(programText[pos] == '=')
        {
            token += programText[pos++];
            return TNotEq;
        }
        else 
        {
            token += programText[pos++];
            printError("invalid token", token);
            return Terr;
        }
    }

    

}

void Scanner::getTextFromFile(const std::string& filename)
{
    std::ifstream input;
	input.open(filename);

	if (!input.is_open())
	{
		const std::string error { "Unable to open file '" + filename + "'" };
		printError(error, "");
        return;
	}

	std::string line;

	while (!input.eof())
	{
		line = "";
		std::getline(input, line);
		programText += line;
	}

	programText += '\0';
	input.close();
}

bool Scanner::isDigit(const char& ch)
{
	return ch >= '0' && ch <= '9';
}

bool Scanner::isLetter(const char& ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool Scanner::isKeyword(const std::string &token)
{
    return keywords.find(token) != keywords.end();
}

void Scanner::printError(const std::string &error, const std::string &token)
{
    std::cerr << "Error: " << error << " '" << token << "'" << std::endl;
}
