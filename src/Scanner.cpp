#include "Scanner.hpp"

Scanner::Scanner(const std::string& filename) {
    reset();
    getTextFromFile(filename);
    buildLineIndex();
}

void Scanner::reset() {
    pos = 0;
    currentLine = 1;
    tokenStartPos = 0;
}

void Scanner::buildLineIndex() {
    lineStarts.clear();
    lineStarts.push_back(0);
    for (uint32_t i = 0; i < programText.length(); ++i) {
        if (programText[i] == '\n') {
            lineStarts.push_back(i + 1);
        }
    }
}

std::string Scanner::getCurrentLineText() const {
    uint32_t lineStart = 0;
    uint32_t lineNum = 1;

    uint32_t errorPos = tokenStartPos;

    for (size_t i = 0; i < lineStarts.size(); ++i) {
        if (lineStarts[i] > errorPos) break;
        lineStart = lineStarts[i];
        lineNum = static_cast<uint32_t>(i + 1);
    }

    uint32_t lineEnd = lineStart;
    while (lineEnd < programText.length() &&
           programText[lineEnd] != '\n' &&
           programText[lineEnd] != '\0') {
        lineEnd++;
    }

    std::string line = programText.substr(lineStart, lineEnd - lineStart);
    return "Строка " + std::to_string(lineNum) + ": " + line;
}

uint16_t Scanner::scan(std::string& token) {
    token.clear();

    while (programText[pos] == ' ' || programText[pos] == '\t' || programText[pos] == '\n') {
        if (programText[pos] == '\n') currentLine++;
        pos++;
    }

    tokenStartPos = pos;

    if (programText[pos] == '\0') {
        token = "End of module";
        return TEnd;
    }

    // numbers
    if (isDigit(programText[pos])) {
        while (isDigit(programText[pos])) token += programText[pos++];

        if (programText[pos] != '.') return TConstInt;

        token += programText[pos++]; // '.'
        if (!isDigit(programText[pos])) {
            printError("некорректная константа", token);
            return Terr;
        }
        while (isDigit(programText[pos])) token += programText[pos++];
        return TConstDouble;
    }

    // identifiers / keywords
    if (isLetter(programText[pos])) {
        while (isLetter(programText[pos]) || isDigit(programText[pos])) token += programText[pos++];
        if (isKeyword(token)) return keywords.at(token);
        return TId;
    }

    // punctuation/operators
    auto ch = programText[pos];

    if (ch == '.') { token += programText[pos++]; return TPoint; }
    if (ch == ',') { token += programText[pos++]; return TComma; }
    if (ch == ';') { token += programText[pos++]; return TSemicolon; }
    if (ch == '(') { token += programText[pos++]; return TLB; }
    if (ch == ')') { token += programText[pos++]; return TRB; }
    if (ch == '{') { token += programText[pos++]; return TLFB; }
    if (ch == '}') { token += programText[pos++]; return TRFB; }

    if (ch == '=') {
        token += programText[pos++];
        if (programText[pos] == '=') { token += programText[pos++]; return TEq; }
        return TEval;
    }

    if (ch == '+') {
        token += programText[pos++];
        if (programText[pos] == '=') { token += programText[pos++]; return TPlusEq; }
        if (programText[pos] == '+') { token += programText[pos++]; return TInc; }
        return TPlus;
    }

    if (ch == '-') {
        token += programText[pos++];
        if (programText[pos] == '=') { token += programText[pos++]; return TMinusEq; }
        if (programText[pos] == '-') { token += programText[pos++]; return TDec; }
        return TMinus;
    }

    if (ch == '/') {
        token += programText[pos++];
        if (programText[pos] == '=') { token += programText[pos++]; return TDivEq; }
        return TDiv;
    }

    if (ch == '*') {
        token += programText[pos++];
        if (programText[pos] == '=') { token += programText[pos++]; return TMultEq; }
        return TMult;
    }

    if (ch == '%') {
        token += programText[pos++];
        if (programText[pos] == '=') { token += programText[pos++]; return TModEq; }
        return TMod;
    }

    if (ch == '>') {
        token += programText[pos++];
        if (programText[pos] == '=') { token += programText[pos++]; return TGE; }
        return TG;
    }

    if (ch == '<') {
        token += programText[pos++];
        if (programText[pos] == '=') { token += programText[pos++]; return TLE; }
        return TL;
    }

    if (ch == '!') {
        token += programText[pos++];
        if (programText[pos] == '=') { token += programText[pos++]; return TNotEq; }
        printError("ожидался символ '='", token);
        return Terr;
    }

    token += programText[pos++];
    printError("недопустимый символ", token);
    return Terr;
}

void Scanner::getTextFromFile(const std::string& filename) {
    std::ifstream input(filename);
    if (!input.is_open()) {
        const std::string err = "Невозможно открыть файл '" + filename + "'";
        printError(err, "");
        programText = "\0";
        return;
    }

    std::string line;
    while (std::getline(input, line)) {
        programText += line;
        programText += '\n';
    }
    programText += '\0';
}

bool Scanner::isDigit(const char& ch) {
    return ch >= '0' && ch <= '9';
}

bool Scanner::isLetter(const char& ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool Scanner::isKeyword(const std::string& token) {
    return keywords.find(token) != keywords.end();
}

void Scanner::printError(const std::string& error, const std::string& token) {
    std::cerr << "\nОшибка: " << error;
    if (!token.empty()) std::cerr << " '" << token << "'";
    std::cerr << "\n" << getCurrentLineText() << std::endl;
}
