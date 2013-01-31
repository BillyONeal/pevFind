//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// zipIt.cpp -- Zips results of scan
#include "pch.hpp"
#include <string>
#include <vector>
#include <list>
#include "zip.h"
#include "zipit.h"
#include "fileData.h"

std::size_t iLongestCommonPrefixLength(const std::vector<std::wstring>& input);
void addAllToZipColonStripped(HZIP zip, const std::vector<std::wstring>& inputSrc);

void zipIt(const std::wstring& fileTarget,const std::list<FileData>& files)
{
    static const char error[] = "Couldn't create zip!";

    //Sanity check
    if (files.empty())
        return;
    std::vector<std::wstring> fileStrings;
    fileStrings.reserve(files.size());
    std::copy(files.begin(), files.end(), back_inserter(fileStrings));

    std::size_t chop = 0;
    if (fileStrings.size() > 1)
         chop = iLongestCommonPrefixLength(fileStrings);
    if (chop)
    {
        while(fileStrings[0][chop] != L'\\') {chop--; };
        chop++;
    }

    //Create the zip
    HZIP hZip = CreateZip(fileTarget.c_str(), NULL);
    if (!hZip)
    throw std::runtime_error(error);

    //Add all files to zip
    if(chop || fileStrings[0][1] != L':')
        for(std::vector<std::wstring>::iterator it = fileStrings.begin(); it != fileStrings.end(); it++)
            ZipAdd(hZip, it->c_str() + chop, it->c_str());
    else
        addAllToZipColonStripped(hZip, fileStrings);

    //Close the zip
    CloseZip(hZip);
}



//Returns the length of the longest common prefix possible of the input strings.
//
//For example:
/*
std::vector vec = {
L"C:\Windows\C",
L"C:\Windows\Roland" ,
L"C:\Win\Blah"}
*/
//
// will return 6. 

/*
This would be the case insensitive version -- unused here
std::size_t longestCommonPrefixLength(const std::vector<std::wstring>& input)
{
    std::size_t chop = 0;
    for(; chop != input[0].length(); chop++)
    {
        std::size_t lastMatch = 1;
        for (; lastMatch != input.size() && input[lastMatch][chop] == input[0][chop]; lastMatch++);
        if (lastMatch != input.size())
            break;
    }
    return chop;
}*/

std::size_t iLongestCommonPrefixLength(const std::vector<std::wstring>& input)
{
    std::size_t chop = 0;
    for(; chop != input[0].length(); chop++)
    {
        std::size_t lastMatch = 1;
        for (; lastMatch != input.size() && towlower(input[lastMatch][chop]) == towlower(input[0][chop]); lastMatch++);
        if (lastMatch != input.size())
            break;
    }
    return chop;
}

void addAllToZipColonStripped(HZIP zip, const std::vector<std::wstring>& inputSrc)
{
    std::vector<std::wstring> dest;
    dest.reserve(inputSrc.size());
    std::copy(inputSrc.begin(), inputSrc.end(), std::back_inserter(dest));
    //The zip paths cannot contain colons. If the common variable is zero, then the
    //files are on different drives. To support this, colons are erased:
    for(std::vector<std::wstring>::iterator it = dest.begin(); it != dest.end(); it++)
        it->erase(it->begin()+1, it->begin()+2);
    for(size_t idx = 0; idx < inputSrc.size(); idx++)
        ZipAdd(zip, dest[idx].c_str(), inputSrc[idx].c_str());
}
