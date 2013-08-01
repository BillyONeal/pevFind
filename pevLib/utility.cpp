//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// utility.cpp -- Provides a bunch of utility functions and classes.

#include "pch.hpp"
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <boost/lexical_cast.hpp>
#include "../LogCommon/Win32Exception.hpp"
#include "../LogCommon/Win32Glue.hpp"
#include "utility.h"

using Instalog::UniqueHandle;

std::wstring loadFileAsString(const std::wstring &fileName)
{
    DWORD trash = 0;
    UniqueHandle fileHandle(CreateFileW(fileName.c_str(),GENERIC_READ,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,NULL,NULL));
    if (!fileHandle.IsOpen())
    {
        Instalog::SystemFacades::Win32Exception::ThrowFromLastError();
    }

    BY_HANDLE_FILE_INFORMATION fileInfo;
    GetFileInformationByHandle(fileHandle.Get(),&fileInfo);
    if (fileInfo.nFileSizeHigh != 0)
    {
        throw std::bad_alloc();
    }

    if (!fileInfo.nFileSizeLow) return L""; //Empty file .. can't access zero length vectors below so making it empty.
    std::vector<BYTE> fileData;
    fileData.resize(fileInfo.nFileSizeLow);
    ReadFile(fileHandle.Get(),fileData.data(),fileInfo.nFileSizeLow,&trash,NULL);
    if (IsTextUnicode(fileData.data(),fileInfo.nFileSizeLow,NULL))
    {
        return std::wstring(reinterpret_cast<wchar_t *>(fileData.data()),fileInfo.nFileSizeLow/sizeof(wchar_t));
    } else
    {
        std::string narrowFileString(reinterpret_cast<char *>(fileData.data()),fileInfo.nFileSizeLow);
        return convertUnicode(narrowFileString);
    }
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
    if (length == 0)
    {
        return std::string();
    }

    std::string result;
    result.resize(length);
    WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,uni.c_str(),static_cast<int>(uni.length()),&result[0],length,"?",&blank);
    return result;
}
std::wstring convertUnicode(const std::string &uni)
{
    INT length;
    length = MultiByteToWideChar(CP_ACP,MB_COMPOSITE,uni.c_str(),static_cast<int>(uni.length()),NULL,NULL);
    if (length == 0)
    {
        return std::wstring();
    }

    std::wstring result;
    result.resize(length);
    MultiByteToWideChar(CP_ACP,MB_COMPOSITE,uni.c_str(),static_cast<int>(uni.length()),&result[0],length);
    return result;
}

#pragma comment(lib, "shlwapi")
std::wstring GetShortPathNameStr(std::wstring longPath)
{
    bool relative = ::PathIsRelativeW(longPath.c_str()) != 0;
    if (!relative)
    {
        longPath.insert(0, L"\\\\?\\");
    }

    DWORD bufferlen = GetShortPathNameW(longPath.c_str(), nullptr, 0);
    if (bufferlen == 0)
    {
        DWORD err = GetLastError();
        std::wstringstream ss;
        ss << L"ERROR(0x" << std::setw(8) << std::setfill(L'0') << std::hex << err << L")";
        return ss.str();
    }

    std::wstring result(bufferlen, L'\0');
    GetShortPathNameW(longPath.c_str(),&result[0],bufferlen);
    if (!relative)
    {
        result.erase(0, 4);
    }

    return result;
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
    std::wstring retVar;
    SYSTEMTIME st;
    FileTimeToSystemTime(&date,&st);
    // YYYY
    retVar.append(boost::lexical_cast<std::wstring >(st.wYear));
    retVar.append(L"-");
    // MM
    if (st.wMonth < 10)
        retVar.append(L"0");
    retVar.append(boost::lexical_cast<std::wstring >(st.wMonth));
    retVar.append(L"-");
    // DD
    if (st.wDay < 10)
        retVar.append(L"0");
    retVar.append(boost::lexical_cast<std::wstring >(st.wDay));
    retVar.append(L" ");
    // HH
    if (st.wHour < 10)
        retVar.append(L"0");
    retVar.append(boost::lexical_cast<std::wstring >(st.wHour));
    retVar.append(L":");
    // MM
    if (st.wMinute < 10)
        retVar.append(L"0");
    retVar.append(boost::lexical_cast<std::wstring >(st.wMinute));
    retVar.append(L":");
    // SS
    if (st.wSecond < 10)
        retVar.append(L"0");
    retVar.append(boost::lexical_cast<std::wstring >(st.wSecond));
    return retVar;
}

const std::wstring getSizeString(unsigned __int64 toConvert)
{
    std::wstring res = boost::lexical_cast<std::wstring,__int64> (toConvert);
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
