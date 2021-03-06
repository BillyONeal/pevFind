﻿//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// utility.cpp -- Provides a bunch of utility functions and classes.

#include "pch.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "utility.h"

std::wstring loadFileAsString(const std::wstring &fileName)
{
    std::wstring fileString;
    { // This scope ensures the vector is destroyed when we want
        HANDLE fileHandle;
        DWORD trash = 0;
        fileHandle = CreateFile(fileName.c_str(),GENERIC_READ,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL);
        if (fileHandle == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Could not open file \"" + convertUnicode(fileName) + "\"");
        BY_HANDLE_FILE_INFORMATION fileInfo;
        GetFileInformationByHandle(fileHandle,&fileInfo);
        if (!fileInfo.nFileSizeLow) return L""; //Empty file .. can't access zero length vectors below so making it empty.
        std::vector<BYTE> fileData;
        fileData.resize(fileInfo.nFileSizeLow);
        ReadFile(fileHandle,&fileData[0],fileInfo.nFileSizeLow,&trash,NULL);
        CloseHandle(fileHandle);
        if (IsTextUnicode(&fileData[0],fileInfo.nFileSizeLow,NULL))
        {
            fileString.assign(reinterpret_cast<wchar_t *>(&fileData[0]),fileInfo.nFileSizeLow/sizeof(wchar_t));
        } else
        {
            std::string narrowFileString(reinterpret_cast<char *>(&fileData[0]),fileInfo.nFileSizeLow);
            fileString.assign(convertUnicode(narrowFileString));
        }
    }
    return fileString;
}
std::vector<std::wstring> loadStringsFromFile(const std::wstring &fileName)
{
    std::vector<std::wstring> files;
    std::wstring fileString(loadFileAsString(fileName));
    std::wstring::iterator beginLine = fileString.begin();
    for(std::wstring::iterator cursor = fileString.begin(); cursor != fileString.end(); cursor++)
    {
        switch(*cursor)
        {
        case L'"':
        case L'\r':
        case L'\n':
            files.push_back(std::wstring(beginLine, cursor));
            while (cursor != fileString.end()) {
                //Nested if prevents null refrence on the end.
                if ( *cursor == L'"' || *cursor == L'\'' || *cursor == L'\r' || *cursor == L'\n')
                    cursor++;
                else
                    break;
            };
            if (cursor == fileString.end()) cursor--;
            beginLine = cursor;
        }
    }
    wchar_t lastChar = (*(fileString.end()-1));
    if (lastChar != L'"' && lastChar != L'\'' && lastChar != L'\r' && lastChar != L'\n')
        files.push_back(std::wstring(beginLine, fileString.end()));
    return files;
}

FILETIME UnixTimeToFileTime(const DWORD &t)
{
    FILETIME retVar;
    unsigned __int64 Int64Member(t);
    Int64Member *= 10000000;
    Int64Member += 116444736000000000;
    retVar.dwHighDateTime = Int64Member >> 32;
    retVar.dwLowDateTime = static_cast<DWORD>(Int64Member & 0x00000000FFFFFFFF);
    return retVar;
}

std::string convertUnicode(const std::wstring &uni)
{
    INT length;
    BOOL blank;
    length = WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,uni.c_str(),static_cast<int>(uni.length()),NULL,NULL,"?",&blank);
    std::vector<char> resultRaw(length);
    WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,uni.c_str(),static_cast<int>(uni.length()),&resultRaw[0],length,"?",&blank);
    std::string result(resultRaw.begin(), resultRaw.end());
    return result;
}
std::wstring convertUnicode(const std::string &uni)
{
    INT length;
    length = MultiByteToWideChar(CP_ACP,MB_COMPOSITE,uni.c_str(),static_cast<int>(uni.length()),NULL,NULL);
    std::vector<wchar_t> resultRaw(length);
    MultiByteToWideChar(CP_ACP,MB_COMPOSITE,uni.c_str(),static_cast<int>(uni.length()),&resultRaw[0],length);
    std::wstring result(resultRaw.begin(), resultRaw.end());
    return result;
}
std::wstring GetShortPathNameStr(std::wstring longPath)
{
    wchar_t tempBuff[MAX_PATH];
    DWORD bufferLen = ::GetShortPathNameW(longPath.c_str(),tempBuff,MAX_PATH);
    if (bufferLen == 0)
    {
        DWORD err = GetLastError();
        bufferLen = swprintf_s(tempBuff, L"ERROR(0x%08X)", err);
        assert(bufferLen != -1);
    }

    return std::wstring(tempBuff, bufferLen);
}
std::wstring& expandEnvironmentString(std::wstring& toExpand)
{
    for(size_t idx=0; idx < toExpand.length(); idx++)
    {
        if (toExpand[idx] == L'%')
        {
            idx++;
            if (toExpand[idx] ==L'%')
                toExpand.erase(idx,1);
            else
            {
                size_t idxEnd = toExpand.find(L'%',idx);
                if (idxEnd == toExpand.npos)
                    throw std::runtime_error("Couldn't expand environment string!");
                std::wstring beforeReplacement(toExpand.substr(idx, idxEnd - idx));
                DWORD len = 0;
                len = GetEnvironmentVariable(beforeReplacement.c_str(),NULL, 0);
                std::vector<wchar_t> buffer(len);
                GetEnvironmentVariable(beforeReplacement.c_str(),&buffer[0],len);
                toExpand.replace(idx-1, idxEnd - idx +2, &buffer[0]);
                idx = idxEnd+1;
            }
        }
    }
    return toExpand;
}
bool operator<(const FILETIME &lhs, const FILETIME &rhs)
{
    union
    {
        FILETIME fTimeStruct;
        unsigned long long raw;
    } tempA, tempB;
    tempA.fTimeStruct = lhs;
    tempB.fTimeStruct = rhs;
    return tempA.raw < tempB.raw;
}
bool operator>(const FILETIME &lhs, const FILETIME &rhs)
{
    union
    {
        FILETIME fTimeStruct;
        unsigned long long raw;
    } tempA, tempB;
    tempA.fTimeStruct = lhs;
    tempB.fTimeStruct = rhs;
    return tempA.raw > tempB.raw;
}
bool operator!=(const FILETIME &lhs, const FILETIME &rhs)
{
    union
    {
        FILETIME fTimeStruct;
        unsigned long long raw;
    } tempA, tempB;
    tempA.fTimeStruct = lhs;
    tempB.fTimeStruct = rhs;
    return tempA.raw != tempB.raw;
}
bool operator==(const FILETIME &lhs, const FILETIME &rhs)
{
    union
    {
        FILETIME fTimeStruct;
        unsigned long long raw;
    } tempA, tempB;
    tempA.fTimeStruct = lhs;
    tempB.fTimeStruct = rhs;
    return tempA.raw == tempB.raw;
}
const FILETIME operator+(const FILETIME &lhs, const FILETIME &rhs)
{
    union
    {
        FILETIME fTimeStruct;
        unsigned long long raw;
    } tempA, tempB, ret;
    tempA.fTimeStruct = lhs;
    tempB.fTimeStruct = rhs;
    ret.raw = tempA.raw + tempB.raw;
    return ret.fTimeStruct;
}
const FILETIME operator+(const FILETIME &lhs, const long long &rhs)
{
    union
    {
        FILETIME fTimeStruct;
        unsigned long long raw;
    } tempA, ret;
    tempA.fTimeStruct = lhs;
    ret.raw = tempA.raw + rhs;
    return ret.fTimeStruct;
}
const FILETIME operator+(const long long &lhs, const FILETIME &rhs)
{
    union
    {
        FILETIME fTimeStruct;
        unsigned long long raw;
    } tempA, ret;
    tempA.fTimeStruct = rhs;
    ret.raw = tempA.raw + lhs;
    return ret.fTimeStruct;
}

std::wstring getDateAsString(const FILETIME &date)
{
    // YYYY-MM-DD HH:MM:SS
    wchar_t result[20];
    SYSTEMTIME st;
    FileTimeToSystemTime(&date,&st);
    int printLen = swprintf_s(result, L"%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return std::wstring(result, std::max(0, printLen));
}

const std::wstring getSizeString(unsigned __int64 toConvert)
{
    wchar_t buff[21];
    int result = swprintf_s(buff, L"%I64u", toConvert);
    std::wstring res(buff, std::max(result, 0));
    for (size_t idx = 1; idx < res.length(); idx++)
    {
        if (!((res.length() - idx) % 3))
        {
            res.insert(idx,L",");
            idx+=3;
        }
    }
    return res;
}

const std::wstring rightPad(const std::wstring& input, size_t targetLength, const wchar_t emptyChar[])
{
    std::wstring results(input);
    results.reserve(targetLength);
    while (results.length() < targetLength)
    {
        results.insert(0,emptyChar);
    }
    return results;
}
