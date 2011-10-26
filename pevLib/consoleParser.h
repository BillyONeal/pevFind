//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// consoleParser.h -- Defines the class responsible for parsing
// pevFind's command line. Implements the recursive-descent
// parser.
#pragma once
#include <string>
#include <vector>
#include <windows.h>

class criterion;

class consoleParser 
{
    //Lexer system
    enum tokenType {
        END = 0,
        BRACKET,
        ENDBRACKET,
        NOT,
        AND,
        OR,
        XOR,
        IFARG,
        ELSEARG,
        REGEX,
        MODIFIER
    };

    struct commandToken {
        std::wstring argument;
        std::wstring option;
        std::wstring fullMatch;
        tokenType type;
        std::wstring& operator()() { return fullMatch; };
    };

    std::vector<commandToken> tokens;
    std::vector<commandToken>::iterator curToken;

    void insertNewTokens(std::vector<commandToken>::iterator& position, const std::wstring& commandLine);
    void tokenize(const std::wstring& commandLine);
    void makeTokens(const std::wstring& commandLine, std::vector<commandToken>& resultVec);
    tokenType makeTypeFromToken(const std::wstring& argument);

    //Recursive-Descent Parser
    std::tr1::shared_ptr<criterion> andParse();
    std::tr1::shared_ptr<criterion> orParse();
    std::tr1::shared_ptr<criterion> xorParse();
    std::tr1::shared_ptr<criterion> exprParse();

    //Helpers for the R-D Parser
    bool isExpressionArgumentType();
    std::tr1::shared_ptr<criterion> createBracket();
    std::tr1::shared_ptr<criterion> createIf();
    void createModifier(std::vector<std::tr1::shared_ptr<criterion> > &results);
    void removeArgument(std::size_t argLength, std::wstring& arg);
    void processFilesArgument(commandToken& token);
    void processLoadlineArgument(commandToken& token);
    unsigned long processUL(commandToken& token);
    unsigned long processUL(std::wstring& numberString);
    long processLong(commandToken& token);
    long processLong(std::wstring& numberString);
    std::wstring& getEndOrOption(commandToken& token) const;
    template <typename hash_t> std::tr1::shared_ptr<criterion> createHashList(commandToken& token, std::size_t hashNameLen);
    template <typename hash_t> std::tr1::shared_ptr<criterion> createHash(commandToken& token, std::size_t hashNameLen);
    void ascendingSorts(commandToken& token);
    void descendingSorts(commandToken& token);
    void parseTypeString(commandToken& token, std::vector<std::tr1::shared_ptr<criterion> > &results);
    void parseNotTypeString(commandToken& token, std::vector<std::tr1::shared_ptr<criterion> > &results);
    void createSize(commandToken& token, std::vector<std::tr1::shared_ptr<criterion> > &results);
    void createDate(commandToken& token, std::vector<std::tr1::shared_ptr<criterion> > &results);
    void parseTypeString(TCHAR const *typeString, std::vector<criterion *>& results);
    
    void processAbsoluteDate( std::wstring& toProcess, FILETIME& lowerBound, FILETIME& upperBound);
    template <typename lowerClass, typename upperClass> 
        void subDateType(std::wstring& token, std::vector<std::tr1::shared_ptr<criterion> > &results);
    void processRelativeDate(std::wstring& token, FILETIME &result);
    std::vector<std::wstring> stripCommandStruct(std::vector<commandToken>::iterator start, std::vector<commandToken>::iterator end);
public:
    std::tr1::shared_ptr<criterion> parseCmdLine(const std::wstring& commandLine);
};


template <typename hash_t> std::tr1::shared_ptr<criterion> consoleParser::createHashList(commandToken& token, std::size_t hashNameLen)
{
    removeArgument(hashNameLen, token.argument);
    std::tr1::shared_ptr<criterion> crit(new hash_t(loadStringsFromFile(getEndOrOption(token))));
    token.argument.clear();
    return crit;
}

template <typename hash_t> std::tr1::shared_ptr<criterion> consoleParser::createHash(commandToken& token, std::size_t hashNameLen)
{
    removeArgument(hashNameLen, token.argument);
    std::tr1::shared_ptr<criterion> result(new hash_t(getEndOrOption(token)));
    token.argument.clear();
    return result;
}

template <typename lowerClass, typename upperClass> 
void consoleParser::subDateType(std::wstring& token, std::vector<std::tr1::shared_ptr<criterion> > &results)
{
    FILETIME lowerBound, upperBound;
    wchar_t type = token[0];
    token.erase(token.begin(), token.begin()+1);
    switch(type)
    {
        // Absolute date arguments
    case L'+':
        processAbsoluteDate(token, lowerBound, upperBound);
        results.push_back(std::tr1::shared_ptr<criterion>(new upperClass(lowerBound)));
        break;
    case L'-':
        processAbsoluteDate(token, lowerBound, upperBound);
        results.push_back(std::tr1::shared_ptr<criterion>(new lowerClass(lowerBound)));
        break;
    case L'=':
        processAbsoluteDate(token, lowerBound, upperBound);
        results.push_back(std::tr1::shared_ptr<criterion>(new upperClass(lowerBound)));
        results.push_back(std::tr1::shared_ptr<criterion>(new lowerClass(upperBound)));
        break;
    case L'!':
        processAbsoluteDate(token, lowerBound, upperBound);
        results.push_back(std::tr1::shared_ptr<criterion>(new lowerClass(upperBound)));
        results.push_back(std::tr1::shared_ptr<criterion>(new upperClass(lowerBound)));
        break;
        // Relative date arguments
    case L'G':
    case L'g':
        processRelativeDate(token, lowerBound);
        results.push_back(std::tr1::shared_ptr<criterion>(new upperClass(lowerBound)));
        break;
    case L'L':
    case L'l':
        processRelativeDate(token, lowerBound);
        results.push_back(std::tr1::shared_ptr<criterion>(new lowerClass(lowerBound)));
        break;
    default:
        throw std::runtime_error("Invalid date argument!");
    }
}
