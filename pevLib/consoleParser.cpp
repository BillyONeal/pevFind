//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// consoleParser.cpp -- Implements the class responsable for
// processing pevFind's commandline. Implements the recursive
// descent parser.

#include "pch.hpp"
#include <stdexcept>
#ifndef NDEBUG
#include <iostream> //These are used to print debugging info to disk.
#include <fstream>
#endif
#include <boost/xpressive/xpressive_static.hpp>
#include <boost/algorithm/string.hpp>
#include "utility.h"
#include "logger.h"
#include "consoleParser.h"
#include "globalOptions.h"
#include "opstruct.h"
#include "regex.h"
#include "FILTER.h"

//Entry point into the parser system
std::shared_ptr<criterion> consoleParser::parseCmdLine(const std::wstring& commandLine)
{
    tokenize(commandLine);
    curToken++; //Skip past the program directory itself
    if (boost::algorithm::iequals(curToken->argument, L"vfind"))
        curToken++;
    std::shared_ptr<criterion> result(andParse());
    if (curToken->type != END)
        throw std::runtime_error("Commandline Syntax Error!!");
    return result;
}
//Initial tokenizing function
void consoleParser::tokenize(const std::wstring& commandLine)
{
    makeTokens(commandLine, tokens);
    commandToken endCap;
    endCap.type = END;
    tokens.push_back(endCap);
    curToken = tokens.begin();
}
//Inserts new tokens (--loadline) to the token stream
void consoleParser::insertNewTokens(std::vector<commandToken>::iterator& position, const std::wstring& commandLine)
{
    std::vector<commandToken> newTokens;
    makeTokens(commandLine, newTokens);
    if (newTokens.size())
    {
        size_t pos = position - tokens.begin(); //Remember the position offset
        tokens.insert(position + 1, newTokens.begin(), newTokens.end()); //Invalidates iterators
        position = tokens.begin() + pos; //Use the position offset to make this iterator valid again
    }
}
void consoleParser::makeTokens(const std::wstring& commandLine, std::vector<commandToken>& resultVec)
{
    using namespace boost::xpressive;
    mark_tag argument(2), option(3);
    wsregex tokenRegex =    (argument= as_xpr(L"{")) | (argument= as_xpr("}")) | as_xpr(L'"') >> (*as_xpr(L'-') >> (argument= as_xpr(L'-') >> +(~set[_s | (set= L'"', L'#', L'{', L'}') ])) >>
        !(as_xpr(L'#') >> (option= +(as_xpr(L'#') >> set[range(L'A', L'Z') | range(L'a', L'z') | range(L'0', L'9')
        | L'#' ]| ~set[L'#'])) >> L'#' | as_xpr(L'"') >> (option= +(L"\"\"" | ~set[L'"'])) >> L'"')) >> L'"' |
                            as_xpr(L'"') >> (argument= +(L"\"\"" |~set[L'"'])) >> L'"' |
                            (*as_xpr(L'-') >> (argument= as_xpr(L'-') >> +(~set[_s | (set= L'"', L'#', L'{', L'}') ])) >>
        !(as_xpr(L'#') >> (option= +(as_xpr(L'#') >> set[range(L'A', L'Z') | range(L'a', L'z') | range(L'0', L'9')
        | L'#' ]| ~set[L'#'])) >> L'#' | as_xpr(L'"') >> (option= +(L"\"\"" | ~set[L'"'])) >> L'"')) |
                            (argument= +~_s);
    wsregex_iterator tokenizedIterator(commandLine.begin(), commandLine.end(), tokenRegex);
    wsregex_iterator end;
    for(; tokenizedIterator != end; tokenizedIterator++)
    {
        commandToken madeTok;
        wsmatch const &curMatch = *tokenizedIterator;
        madeTok.argument = curMatch[argument];
        madeTok.option = curMatch[option];
        madeTok.type = makeTypeFromToken(curMatch[argument]);
        madeTok.fullMatch = curMatch[0];
        resultVec.push_back(madeTok);
    }
}
consoleParser::tokenType consoleParser::makeTypeFromToken(const std::wstring& argument)
{
    if (argument == L"{")
        return BRACKET;
    if (argument == L"}")
        return ENDBRACKET;
    if (argument.length() == 2)
    {
        if    (((argument[0] == L'O') || (argument[0] == L'o'))
            && ((argument[1] == L'R') || (argument[1] == L'r')))
            return OR;
        if    (((argument[0] == L'I') || (argument[0] == L'i'))
            && ((argument[1] == L'F') || (argument[1] == L'f')))
            return IFARG;
    }
    if (argument.length() == 3)
    {
        //And
        if (   ((argument[0] == L'A') || (argument[0] == L'a'))
            && ((argument[1] == L'N') || (argument[1] == L'n'))
            && ((argument[2] == L'D') || (argument[2] == L'd')) )
            return AND;
        if (   ((argument[0] == L'N') || (argument[0] == L'n'))
            && ((argument[1] == L'O') || (argument[1] == L'o'))
            && ((argument[2] == L'T') || (argument[2] == L't')) )
            return NOT;
        //Xor
        if (   ((argument[0] == L'X') || (argument[0] == L'x'))
            && ((argument[1] == L'O') || (argument[1] == L'o'))
            && ((argument[2] == L'R') || (argument[2] == L'r')) )
            return XOR;
    }
    //Else
    if ( argument.length() == 4
        && ((argument[0] == L'E') || (argument[0] == L'e'))
        && ((argument[1] == L'L') || (argument[1] == L'l'))
        && ((argument[2] == L'S') || (argument[2] == L's'))
        && ((argument[3] == L'E') || (argument[3] == L'e')) )
        return ELSEARG;
    if (argument[0] == L'-')
        return MODIFIER;
    return REGEX;
}
std::shared_ptr<criterion> consoleParser::andParse()
{
    std::shared_ptr<criterion> prev(orParse());
    while (curToken->type == AND)
    {
        curToken++;
        std::shared_ptr<criterion> cur(orParse());
        std::shared_ptr<criterion> newNode(new andAndClass(cur, prev));
        prev = newNode;
    }
    return prev;
}
std::shared_ptr<criterion> consoleParser::orParse()
{
    std::shared_ptr<criterion> prev(xorParse());
    while (curToken->type == OR)
    {
        curToken++;
        std::shared_ptr<criterion> cur(xorParse());
        std::shared_ptr<criterion> newNode(new orAndClass(cur, prev));
        prev = newNode;
    }
    return prev;
}
std::shared_ptr<criterion> consoleParser::xorParse()
{
    std::shared_ptr<criterion> prev(exprParse());
    while (curToken->type == XOR)
    {
        curToken++;
        std::shared_ptr<criterion> cur(exprParse());
        std::shared_ptr<criterion> newNode(new xorAndClass(cur, prev));
        prev = newNode;
    }
    return prev;
}
std::shared_ptr<criterion> consoleParser::exprParse()
{
    std::vector<std::shared_ptr<criterion> > results;
    while (isExpressionArgumentType())
    {
        switch(curToken->type)
        {
        case NOT:
            {
                curToken++;
                std::shared_ptr<criterion> ptrVal(new notAndClass(exprParse()));
                results.push_back(ptrVal);
            }
            break;
        case BRACKET:
            results.push_back(createBracket());
            curToken++;
            break;
        case IFARG:
            results.push_back(createIf());
            curToken++;
            break;
        case REGEX:
            {
                std::shared_ptr<regexClass> ptrVal(new vFindRegex(curToken->argument));
                results.push_back(ptrVal);
                globalOptions::regularExpressions.push_back(ptrVal);
                curToken++;
            }
            break;
        case MODIFIER:
            createModifier(results);
            curToken++;
            break;
        default:
            throw std::runtime_error("Token unexpected. Check expression!");
        }
    }
    switch(results.size())
    {
    case 0:
        throw std::runtime_error("Expression resulted in zero qualifiers. Check expression.");
    case 1:
        return results[0];
    default:
        return std::shared_ptr<criterion>( new bracketClass(results) );
    }
}
std::shared_ptr<criterion> consoleParser::createBracket()
{
    curToken++;
    std::shared_ptr<criterion> result = andParse();
    if (curToken->type != ENDBRACKET)
        throw std::runtime_error("Unbalanced {}s in your expression.");
    return result;
}
std::shared_ptr<criterion> consoleParser::createIf()
{
    curToken++;
    std::shared_ptr<criterion> condition(exprParse());
    std::shared_ptr<criterion> then(exprParse());
    std::shared_ptr<criterion> els((criterion *)NULL);
    if (curToken->type == ELSEARG)
    {
        curToken++;
        els = exprParse();
    }
    std::shared_ptr<criterion> result(new ifClass(condition, then, els));
    return result;
}
bool consoleParser::isExpressionArgumentType()
{
    switch(curToken->type)
    {
    case BRACKET:
    case IFARG:
    case REGEX:
    case MODIFIER:
    case NOT:
        return true;
    }
    return false;
}
void consoleParser::createModifier(std::vector<std::shared_ptr<criterion> > &results)
{
    using namespace boost::algorithm;
    commandToken token = *curToken;
    token.argument.erase(token.argument.begin(), token.argument.begin() + 1);
    do {
        if(      istarts_with(token.argument, L"custom"))
        {
            globalOptions::displaySpecification = token.option;    
            return;
        }
        else if (istarts_with(token.argument, L"c"))
        {
            globalOptions::displaySpecification = token.option;
            return;
        }
        else if (istarts_with(token.argument, L"debug"))
        {
            token.argument.erase(0, 5);
            globalOptions::debug = true;
        }
        else if (istarts_with(token.argument, L"d"))
            createDate(token, results);
        else if (istarts_with(token.argument, L"encodingacp"))
        {
            token.argument.erase(0, 11);
            globalOptions::encoding = globalOptions::ENCODING_TYPE_ACP;
        }
        else if (istarts_with(token.argument, L"encodingoem"))
        {
            token.argument.erase(0, 11);
            globalOptions::encoding = globalOptions::ENCODING_TYPE_OEM;
        }
        else if (istarts_with(token.argument, L"encodingutf16"))
        {
            token.argument.erase(0, 13);
            globalOptions::encoding = globalOptions::ENCODING_TYPE_UTF16;
        }
        else if (istarts_with(token.argument, L"encodingutf8"))
        {
            token.argument.erase(0, 12);
            globalOptions::encoding = globalOptions::ENCODING_TYPE_UTF8;
        }
        else if (istarts_with(token.argument, L"enable-filesystem-redirector-64"))
        {
            token.argument.erase(0, 31);
            globalOptions::disable64Redirector = false;
        }
        else if (istarts_with(token.argument, L"ea"))
        {
            token.argument.erase(0, 2);
            globalOptions::encoding = globalOptions::ENCODING_TYPE_ACP;
        }
        else if (istarts_with(token.argument, L"eo"))
        {
            token.argument.erase(0, 2);
            globalOptions::encoding = globalOptions::ENCODING_TYPE_OEM;
        }
        else if (istarts_with(token.argument, L"eu8"))
        {
            token.argument.erase(0, 3);
            globalOptions::encoding = globalOptions::ENCODING_TYPE_UTF8;
        }
        else if (istarts_with(token.argument, L"eu16"))
        {
            token.argument.erase(0, 4);
            globalOptions::encoding = globalOptions::ENCODING_TYPE_UTF16;
        }
        else if (istarts_with(token.argument, L"expand"))
        {
            token.argument.erase(0, 6);
            globalOptions::expandRegex = true;
        }
        else if (istarts_with(token.argument, L"ex"))
        {
            token.argument.erase(0, 2);
            globalOptions::expandRegex = true;
        }
        else if (istarts_with(token.argument, L"e"))
        {
            token.argument.erase(0, 1);
            globalOptions::displaySpecification = L"[#p] #f";
        }
        else if (istarts_with(token.argument, L"filelook"))
        {
            token.argument.erase(0, 9);
            globalOptions::displaySpecification = L"---- #f ----#nCompany: #d#nFile Description: #e#nFile Version: #g #nProduct Name: #i#nCopyright: #j#nOriginal file name: #k#nFile Size: #u#nCreated Time: #c #nModified Time: #m#nAccessed Time: #a#nMD5: #5#nSHA1: #1#nSHA224: #2#nSHA256: #3#nSHA384: #4#nSHA512: #6";
        }
        else if (istarts_with(token.argument, L"files"))
        {
            std::shared_ptr<regexClass> it(new filesRegexPlaceHolder());
            results.push_back(it);
            globalOptions::regularExpressions.push_back(it);
            processFilesArgument(token);
        }
        else if (istarts_with(token.argument, L"fs32"))
        {
            token.argument.erase(0, 4);
            globalOptions::disable64Redirector = false;
        }
        else if (istarts_with(token.argument, L"full"))
        {
            token.argument.erase(0, 4);
            globalOptions::fullPath = true;
        }
        else if (istarts_with(token.argument, L"f"))
        {
            token.argument.erase(0, 1);
            globalOptions::fullPath = true;
        }
        else if (istarts_with(token.argument, L"k"))
        {
            token.argument.erase(0, 1);
            globalOptions::killProc = true;
        }
        else if (istarts_with(token.argument, L"loadline"))
            processLoadlineArgument(token);
        else if (istarts_with(token.argument, L"limit"))
        {
            removeArgument(5, token.argument);
            globalOptions::lineLimit = processUL(token);
        }
        else if (istarts_with(token.argument, L"long"))
        {
            token.argument.erase(0, 4);
            if (!boost::algorithm::equals(globalOptions::displaySpecification, L"#8"))
                globalOptions::displaySpecification = L"#t #s #m  #f";
            else
                globalOptions::displaySpecification = L"#t #s #m  #8";
            globalOptions::summary = true;
        }
        else if (istarts_with(token.argument, L"l"))
        {
            token.argument.erase(0, 1);
            if (!boost::algorithm::equals(globalOptions::displaySpecification, L"#8"))
                globalOptions::displaySpecification = L"#t #s #m  #f";
            else
                globalOptions::displaySpecification = L"#t #s #m  #8";
            globalOptions::summary = true;
        }
        else if (istarts_with(token.argument, L"md5list"))
            results.push_back(createHashList<md5List>(token, 7));
        else if (istarts_with(token.argument, L"md5elist"))
            results.push_back(createHashList<md5EList>(token, 8));
        else if (istarts_with(token.argument, L"md5"))
            results.push_back(createHash<md5Match>(token, 3));
        else if (istarts_with(token.argument, L"m"))
        {
            token.argument.erase(0, 1);
            if (!boost::algorithm::equals(globalOptions::displaySpecification, "#t #s #m  #f"))
                globalOptions::displaySpecification = L"#8";
            else
                globalOptions::displaySpecification = L"#t #s #m  #8";
        }
        else if (istarts_with(token.argument, L"norecursion"))
        {
            token.argument.erase(0, 11);
            globalOptions::noSubDirectories = true;
        }
        else if (istarts_with(token.argument, L"nrvf"))
        {
            token.argument.clear();
            boost::algorithm::replace_all(token.option, L"##", L"#");
            std::shared_ptr<regexClass> ptrVal(new vFindRegex(token.option, false));
            results.push_back(ptrVal);
            globalOptions::regularExpressions.push_back(ptrVal);
        }
        else if (istarts_with(token.argument, L"n"))
        {
            token.argument.erase(0, 1);
            globalOptions::summary = true;
        }
        else if (istarts_with(token.argument, L"output"))
        {
            removeArgument(6, token.argument);
            logger.update(getEndOrOption(token));
            token.argument.clear();
        }
        else if (istarts_with(token.argument, L"peinfo"))
        {
            token.argument.erase(0, 6);
            globalOptions::displaySpecification = L"[#p] #f";
        }
        else if (istarts_with(token.argument, L"preg"))
        {
            token.argument.clear();
            boost::algorithm::replace_all(token.option, L"##", L"#");
            results.push_back(std::shared_ptr<regexClass>(new perlRegex(token.option)));
        }
        else if (istarts_with(token.argument, L"r"))
        {
            token.argument.erase(0, 1);
            globalOptions::noSubDirectories = true;
        }
        else if (istarts_with(token.argument, L"sa"))
            ascendingSorts(token);
        else if (istarts_with(token.argument, L"sd"))
            descendingSorts(token);
        else if (istarts_with(token.argument, L"sha1list"))
            results.push_back(createHashList<sha1List>(token, 8));
        else if (istarts_with(token.argument, L"sha1elist"))
            results.push_back(createHashList<sha1EList>(token, 9));
        else if (istarts_with(token.argument, L"sha1"))
            results.push_back(createHash<md5Match>(token, 4));
        else if (istarts_with(token.argument, L"short"))
        {
            token.argument.erase(0, 5);
            globalOptions::displaySpecification = L"#8";
        }
        else if (istarts_with(token.argument, L"summary"))
        {
            token.argument.erase(0, 7);
            globalOptions::summary = true;
        }
        else if (istarts_with(token.argument, L"skip"))
        {
            removeArgument(4, token.argument);
            results.push_back(std::shared_ptr<criterion>(new skipper(getEndOrOption(token))));
            token.argument.clear();
        }
        else if (istarts_with(token.argument, L"s"))
            createSize(token,results);
        else if (istarts_with(token.argument, L"timeout"))
        {
            removeArgument(7, token.argument);
            globalOptions::timeout = processUL(token);
        }
        else if (istarts_with(token.argument, L"tx"))
        {
            removeArgument(2, token.argument);
            globalOptions::timeout = processUL(token);
        }
        else if (istarts_with(token.argument, L"t!"))
        {
            removeArgument(2, token.argument);
            parseNotTypeString(token, results);
        }
        else if (istarts_with(token.argument, L"t"))
        {
            removeArgument(1, token.argument);
            parseTypeString(token, results);
        }
        else if (istarts_with(token.argument, L"zip"))
        {
            removeArgument(3, token.argument);
            globalOptions::zipFileName = getEndOrOption(token);
            return;
        }
        else
            throw std::runtime_error("Invalid modifier.");
    } while (!token.argument.empty());
}
void consoleParser::removeArgument(std::size_t argLength, std::wstring& arg)
{
    std::wstring::iterator it = arg.begin() + argLength;
    while(it != arg.end() && *it == L':') { it++; };
    arg.erase(arg.begin(), it);
}
void consoleParser::parseTypeString(commandToken& token, std::vector<std::shared_ptr<criterion> > &results)
{
    std::wstring::iterator it = token.argument.begin();
    bool foundType = true;
    do {
        switch (*it)
        {
        case L'A':
        case L'a':
            results.push_back(std::shared_ptr<criterion>(new isArchive()));
            break;
        case L'B':
        case L'b':
            results.push_back(std::shared_ptr<criterion>(new isPEPlusFile()));
            break;
        case L'C':
        case L'c':
            results.push_back(std::shared_ptr<criterion>(new isCompressed()));
            break;
        case L'D':
        case L'd':
            results.push_back(std::shared_ptr<criterion>(new isDirectory()));
            break;
        case L'E':
        case L'e':
            results.push_back(std::shared_ptr<criterion>(new isReparsePoint()));
            break;
        case L'F':
        case L'f':
            results.push_back(std::shared_ptr<criterion>(new isFile()));
            break;
        case L'G':
        case L'g':
            results.push_back(std::shared_ptr<criterion>(new sigIsValid()));
            break;
        case L'J':
        case L'j':
            results.push_back(std::shared_ptr<criterion>(new timestampValid()));
            break;
        case L'K':
        case L'k':
            results.push_back(std::shared_ptr<criterion>(new checkSumValid()));
            break;
        case L'L':
        case L'l':
            results.push_back(std::shared_ptr<criterion>(new isDLLFile()));
            break;
        case L'O':
        case L'o':
            results.push_back(std::shared_ptr<criterion>(new isSFCProtected()));
            break;
        case L'P':
        case L'p':
            if (boost::istarts_with(boost::iterator_range<std::wstring::iterator>(it, token.argument.end()), L"pne"))
            {
                results.push_back(std::shared_ptr<criterion>(new isNEFile()));
                it += 2;
            } else if (boost::istarts_with(boost::iterator_range<std::wstring::iterator>(it, token.argument.end()), L"ple"))
            {
                results.push_back(std::shared_ptr<criterion>(new isLEFile()));
                it += 2;
            } else if (boost::istarts_with(boost::iterator_range<std::wstring::iterator>(it, token.argument.end()), L"pmz"))
            {
                results.push_back(std::shared_ptr<criterion>(new isMZFile()));
                it += 2;
            } else if (boost::istarts_with(boost::iterator_range<std::wstring::iterator>(it, token.argument.end()), L"p2"))
            {
                results.push_back(std::shared_ptr<criterion>(new is2ExecFile()));
                it++;
            } else if (boost::istarts_with(boost::iterator_range<std::wstring::iterator>(it, token.argument.end()), L"p64"))
            {
                results.push_back(std::shared_ptr<criterion>(new isPEPlusFile()));
                it += 2;
            } else
            {
                results.push_back(std::shared_ptr<criterion>(new isPEFile()));
            }
            break;
        case L'H':
        case L'h':
            results.push_back(std::shared_ptr<criterion>(new isHidden()));
            break;
        case L'R':
        case L'r':
            results.push_back(std::shared_ptr<criterion>(new isReadOnly()));
            break;
        case L'S':
        case L's':
            results.push_back(std::shared_ptr<criterion>(new isSystem()));
            break;
        case L'V':
        case L'v':
            results.push_back(std::shared_ptr<criterion>(new isVolumeLabel()));
            break;
        case L'W':
        case L'w':
            results.push_back(std::shared_ptr<criterion>(new isWritable()));
            break;
        default:
            foundType = false;
            it--;
        }
        it++;
    } while (foundType && it != token.argument.end());
    token.argument.erase(token.argument.begin(), it);
}

void consoleParser::createDate(commandToken& token, std::vector<std::shared_ptr<criterion> > &results)
{
    removeArgument(1, token.argument);
    switch (token.argument[0])
    {
    case L'C':
    case L'c':
        removeArgument(1, token.argument);
        subDateType<createdLDate, createdGDate>(getEndOrOption(token), results);
        break;
    case L'A':
    case L'a':
        removeArgument(1, token.argument);
        subDateType<accessLDate, accessGDate>(getEndOrOption(token), results);
        break;
    case L'H':
    case L'h':
        removeArgument(1, token.argument);
        subDateType<headerLDate, headerGDate>(getEndOrOption(token), results);
    case L'M':
    case L'm':
        token.argument.erase(0, 1);
    default:
        if (!token.argument.empty())
            removeArgument(0, token.argument); //Removes the : if present
        subDateType<modifiedLDate, modifiedGDate>(getEndOrOption(token), results);
    }
}
void consoleParser::processAbsoluteDate( std::wstring& toProcess, FILETIME& lowerBound, FILETIME& upperBound)
{
    long        v;
    SYSTEMTIME now;
    SYSTEMTIME st;
    int significantFigures = 0;
    std::wstring::iterator cursor = toProcess.begin();

    // Get the default time (which is now)
    GetLocalTime(&now);

    // Parse the date spec

    // Get year '[CC]YY' part
    memset(&st, 0, sizeof(st));
    while (cursor != toProcess.end() && !iswdigit(*cursor))
        cursor++;
    if (cursor != toProcess.end() && iswdigit(*cursor))
    {
        v = 0;
        for (int i = 0;  i < 4 && iswdigit(*cursor);  i++, cursor++)
            v = v*10 + (*cursor - L'0');

        if (v < 100)
            v += 1900;

        st.wYear =   static_cast<WORD>(v);
        st.wMonth =  1;
        st.wDay =    1;
        st.wHour =   0;
        st.wMinute = 0;
        st.wSecond = 0;
        significantFigures = 1;
    }
    else
        st.wYear = now.wYear;

    // Get the month 'MM' part
    while (cursor != toProcess.end() && !iswdigit(*cursor))
        cursor++;
    if (cursor != toProcess.end() && iswdigit(*cursor))
    {
        v = 0;
        for (int i = 0;  i < 2 && iswdigit(*cursor);  i++, cursor++)
            v = v*10 + *cursor - L'0';

        st.wMonth = static_cast<WORD>(v);
        st.wDay =   1;
        significantFigures = 2;
    }

    // Get the day 'DD' part
    while (cursor != toProcess.end() && !iswdigit(*cursor))
        cursor++;
    if (cursor != toProcess.end() && isdigit(*cursor))
    {
        v = 0;
        for (int i = 0;  i < 2 && iswdigit(*cursor);  i++, cursor++)
            v = v*10 + *cursor - L'0';

        st.wDay = static_cast<WORD>(v);
        significantFigures = 3;
    }

    // Get the hour 'HH' part
    while (cursor != toProcess.end() && !iswdigit(*cursor))
        cursor++;
    if (cursor != toProcess.end() && isdigit(*cursor))
    {
        v = 0;
        for (int i = 0;  i < 2 && iswdigit(*cursor);  i++, cursor++)
            v = v*10 + *cursor - L'0';

        st.wHour =   static_cast<WORD>(v);
        st.wMinute = 0;
        st.wSecond = 0;
        significantFigures = 4;
    }

    // Get the minute 'MM' part
    while (cursor != toProcess.end() && !iswdigit(*cursor))
        cursor++;
    if (cursor != toProcess.end() && isdigit(*cursor))
    {
        v = 0;
        for (int i = 0;  i < 2 && iswdigit(*cursor);  i++, cursor++)
            v = v*10 + *cursor - L'0';

        st.wMinute = static_cast<WORD>(v);
        st.wSecond = 0;
        significantFigures = 5;
    }

    // Get the second 'SS' part
    while (cursor != toProcess.end() && !iswdigit(*cursor))
        cursor++;
    if (cursor != toProcess.end() && isdigit(*cursor))
    {
        v = 0;
        for (int i = 0;  i < 2 && iswdigit(*cursor);  i++, cursor++)
            v = v*10 + *cursor - L'0';

        st.wSecond = static_cast<WORD>(v);
        significantFigures = 6;
    }

    toProcess.erase(toProcess.begin(), cursor);

    if (!SystemTimeToFileTime(&st, &lowerBound))
        throw std::runtime_error("Invalid date.");

    switch(significantFigures)
    {
    case 1:
        //One year in 100ns intervals
        upperBound = lowerBound + 315360000000000ull;
        break;
    case 2:
        //One month in 100ns intervals
        upperBound = lowerBound + 25920000000000ull;
        break;
    case 3:
        //One day
        upperBound = lowerBound + 864000000000ull;
        break;
    case 4:
        //One hour
        upperBound = lowerBound + 36000000000ull;
        break;
    case 5:
        //One minnute
        upperBound = lowerBound + 36000000000ull;
        break;
    case 6:
    default:
        upperBound = lowerBound  + 1000000ull;
        break;
    }
}
void consoleParser::processRelativeDate(std::wstring& token, FILETIME &result)
{
    //Get the current time as an __int64
    //The result variable is used as a temporary here..
    //it will be overwritten with the correct value later.
    GetSystemTimeAsFileTime(&result);
    unsigned long long numericalTime = (static_cast<unsigned long long>(result.dwHighDateTime) << 32) + static_cast<unsigned long long>(result.dwLowDateTime);

    //Get the user's number
    unsigned long long userValue = processLong(token);

    //Check for all the possible suffixes
    switch (token[0])
    {
    case L'y':
    case L'Y':
        numericalTime -= 315360000000000ull * userValue;
        break;
    case L'm':
    case L'M':
        switch (token[1])
        {
        case L'm':
        case L'M':
            token.erase(token.begin(), token.begin() + 1);
            numericalTime -= 600000000ull * userValue;
            break;
        default:
            numericalTime -= 25920000000000ull * userValue;
        }
        break;
    case L'H':
    case L'h':
        numericalTime -= 36000000000ull * userValue;
        break;
    case L'S':
    case L's':
        numericalTime -= 10000000ull * userValue;
        break;
    case L'd':
    case L'D':
    case NULL:
    default:
        numericalTime -= 864000000000ull * userValue;
        break;
    }

    //Erase the area which defined the date and suffix pair from
    //the source string.
    if (token.size())
        token.erase(token.begin(), token.begin() + 1);

    //Convert the numerical time back into a FILETIME structure.
    result.dwHighDateTime = numericalTime >> 32;
    result.dwLowDateTime = numericalTime & 0x00000000FFFFFFFFull;
}
unsigned long consoleParser::processUL(commandToken& token)
{
    return processUL(token.argument);
}
unsigned long consoleParser::processUL(std::wstring& numberString)
{
    //Setup begin/end pointers
    wchar_t const * beginPoint;
    wchar_t const * endPoint;
    beginPoint = numberString.c_str();
    endPoint = beginPoint + wcslen(beginPoint);

    //Get the number
    //The const cast here is because the compiler doesn't correctly realize that we are refering to const data, not a const pointer.
    //The function does not modify the data, so this is okay.
    unsigned long ul = wcstoul(beginPoint, const_cast<wchar_t **>(&endPoint), 10);

    //Erase the number
    numberString.erase(0, endPoint - beginPoint);

    return ul;
}
long consoleParser::processLong(commandToken& token)
{
    return processLong(token.argument);
}
long consoleParser::processLong(std::wstring& numberString)
{
    //Setup begin/end pointers
    wchar_t const * beginPoint;
    wchar_t const * endPoint;
    beginPoint = numberString.c_str();
    endPoint = beginPoint + wcslen(beginPoint);

    //Get the number
    //The const cast here is because the compiler doesn't correctly realize that we are refering to const data, not a const pointer.
    //The function does not modify the data, so this is okay.
    long ul = wcstol(beginPoint, const_cast<wchar_t **>(&endPoint), 10);

    //Erase the number
    numberString.erase(0, endPoint - beginPoint);

    return ul;
}

void consoleParser::processFilesArgument(commandToken &token)
{
    removeArgument(5, token.argument);
    std::vector<std::wstring> fileStrings = loadStringsFromFile(getEndOrOption(token));
    globalOptions::fileList.insert(globalOptions::fileList.end(),fileStrings.begin(), fileStrings.end());
    token.argument.clear();
}

std::wstring& consoleParser::getEndOrOption(commandToken& token) const
{
    if (token.argument.empty())
        return token.option;
    return token.argument;
}

void consoleParser::processLoadlineArgument(commandToken& token)
{
    removeArgument(8, token.argument);
    std::wstring newCommands(loadFileAsString(getEndOrOption(token)));
#ifndef NDEBUG
    std::wofstream out(L"C:\\command.txt", std::ios::app | std::ios::out);
    out << L"LINELOADED\r\n" << newCommands << L"\r\nENDLINELOADED\r\n";
    out.close();
#endif
    insertNewTokens(curToken, newCommands);
    token.argument.clear();
}

void consoleParser::ascendingSorts(commandToken& token)
{
    using namespace boost::algorithm;
    removeArgument(2, token.argument);
    if      (istarts_with(token.argument, L"size"))
    {
        token.argument.erase(0, 4);
        globalOptions::addSort(globalOptions::SIZE);
    }
    else if (istarts_with(token.argument, L"date"))
    {
        token.argument.erase(0, 4);
        globalOptions::addSort(globalOptions::MDATE);
    }
    else if (istarts_with(token.argument, L"adate"))
    {
        token.argument.erase(0, 5);
        globalOptions::addSort(globalOptions::ADATE);
    }
    else if (istarts_with(token.argument, L"cdate"))
    {
        token.argument.erase(0, 5);
        globalOptions::addSort(globalOptions::CDATE);
    }
    else if (istarts_with(token.argument, L"hdate"))
    {
        token.argument.erase(0, 5);
        globalOptions::addSort(globalOptions::HDATE);
    }
    else if (istarts_with(token.argument, L"mdate"))
    {
        token.argument.erase(0, 5);
        globalOptions::addSort(globalOptions::MDATE);
    }
    else if (istarts_with(token.argument, L"name"))
    {
        token.argument.erase(0, 4);
        globalOptions::addSort(globalOptions::NAME);
    }
    else if (istarts_with(token.argument, L"iname"))
    {
        token.argument.erase(0, 5);
        globalOptions::addSort(globalOptions::INAME);
    } else
        throw std::runtime_error("Invalid sort!");

}

void consoleParser::descendingSorts(commandToken& token)
{
    removeArgument(2, token.argument);
    using namespace boost::algorithm;
    if      (istarts_with(token.argument, L"size"))
    {
        token.argument.erase(0, 4);
        globalOptions::addSort(globalOptions::DSIZE);
    }
    else if (istarts_with(token.argument, L"date"))
    {
        token.argument.erase(0, 4);
        globalOptions::addSort(globalOptions::DMDATE);
    }
    else if (istarts_with(token.argument, L"adate"))
    {
        token.argument.erase(0, 5);
        globalOptions::addSort(globalOptions::DADATE);
    }
    else if (istarts_with(token.argument, L"cdate"))
    {
        token.argument.erase(0, 5);
        globalOptions::addSort(globalOptions::DCDATE);
    }
    else if (istarts_with(token.argument, L"hdate"))
    {
        token.argument.erase(0, 5);
        globalOptions::addSort(globalOptions::DHDATE);
    }
    else if (istarts_with(token.argument, L"mdate"))
    {
        token.argument.erase(0, 5);
        globalOptions::addSort(globalOptions::DMDATE);
    }
    else if (istarts_with(token.argument, L"name"))
    {
        token.argument.erase(0, 4);
        globalOptions::addSort(globalOptions::DNAME);
    }
    else if (istarts_with(token.argument, L"iname"))
    {
        token.argument.erase(0, 5);
        globalOptions::addSort(globalOptions::DINAME);
    } else
        throw std::runtime_error("Invalid sort!");
}

void consoleParser::parseNotTypeString(commandToken& token, std::vector<std::shared_ptr<criterion> > &results)
{
    std::vector<std::shared_ptr<criterion> > resTmp;
    parseTypeString(token, resTmp);
    if (resTmp.size())
    {
        for (std::vector<std::shared_ptr<criterion> >::iterator it = resTmp.begin(); it != resTmp.end(); it++)
            results.push_back( std::shared_ptr<criterion>(new notAndClass(*it)) );
    }
    
}
void consoleParser::createSize(commandToken& token, std::vector<std::shared_ptr<criterion> > &results)
{
    removeArgument(1, token.argument);
    wchar_t typeChar = token.argument[0];
    if (      typeChar == L'+' || typeChar == L'G' || typeChar == L'g' )
    {
        removeArgument(1, token.argument);
        results.push_back(std::shared_ptr<criterion>( new gtSizeFilter(processUL(token))));
    } else if ( typeChar == L'-' || typeChar == L'L' || typeChar == L'l' )
    {
        removeArgument(1, token.argument);
        results.push_back(std::shared_ptr<criterion>( new ltSizeFilter(processUL(token))));
    } else if ( typeChar == L'=' )
    {
        removeArgument(1, token.argument);
        results.push_back(std::shared_ptr<criterion>( new isSizeFilter(processUL(token))));
    } else if ( typeChar == L'!' )
    {
        removeArgument(1, token.argument);
        results.push_back(std::shared_ptr<criterion>( new notSizeFilter(processUL(token))));
    } else //Number range
    {
        __int64 a = processUL(token);
        token.argument.erase(0, 1);
        __int64 b = processUL(token);
        if (a > b)
            std::swap(a, b);
        results.push_back(std::shared_ptr<criterion>( new gtSizeFilter(a - 1)));
        results.push_back(std::shared_ptr<criterion>( new ltSizeFilter(b + 1)));
    }
}
