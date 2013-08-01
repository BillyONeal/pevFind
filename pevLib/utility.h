#ifndef _UTILITY_H_INCLUDED
#define _UTILITY_H_INCLUDED
//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// utility.h -- Provides a bunch of utility functions and classes.

#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "globalOptions.h"

class FileData;
std::wstring getDateAsString(const FILETIME &date);
const std::wstring getSizeString(unsigned __int64 toConvert);
const std::wstring rightPad(const std::wstring& input, size_t targetLength, const wchar_t emptyChar[] = L" ");

static class _disable64
{
    typedef BOOL (WINAPI *_disableWow64)(LPVOID *);
    typedef BOOL (WINAPI *_revertWow64)(LPVOID);
    typedef BOOL (WINAPI *_isWow64Process) (HANDLE, PBOOL);
    _revertWow64 revertWow64;
    _isWow64Process isWow64Process;
    _disableWow64 disableWow64;
    bool disabled;
    bool functionsExist;
    LPVOID oldValue;
    HMODULE kernel32;
public:
    _disable64()
    {
        functionsExist = true;
        BOOL is64ReturnValue;    
        kernel32 = LoadLibrary(L"kernel32.dll");
        if (!kernel32)
        {
            functionsExist = false;
            return;
        }
        isWow64Process = (_isWow64Process) GetProcAddress(kernel32,"IsWow64Process");
        if (!isWow64Process)
        {
            functionsExist = false;
            return;
        }
        isWow64Process(GetCurrentProcess(),&is64ReturnValue);
        if (!is64ReturnValue)
        {
            functionsExist = false;
            FreeLibrary(kernel32);
            return;
        }
        disableWow64 = (_disableWow64) GetProcAddress(kernel32,"Wow64DisableWow64FsRedirection");
        if (!disableWow64)
        {
            functionsExist = false;
            return;
        }
        revertWow64 = (_revertWow64) GetProcAddress(kernel32,"Wow64RevertWow64FsRedirection");
        if (!revertWow64)
        {
            functionsExist = false;
            return;
        }
    }
    ~_disable64()
    {
        if (functionsExist)
            FreeLibrary(kernel32);
    }
    void disableFS()
    {
        if (!functionsExist)
            return;
        if (!globalOptions::disable64Redirector)
            return;
        disableWow64(&oldValue);
        disabled = true;
    };
    void enableFS()
    {
        if (!disabled)
            return;
        revertWow64(oldValue);
        disabled = false;
    };
} disable64;

bool operator<(const FILETIME &lhs, const FILETIME &rhs);
bool operator>(const FILETIME &lhs, const FILETIME &rhs);
bool operator!=(const FILETIME &lhs, const FILETIME &rhs);
bool operator==(const FILETIME &lhs, const FILETIME &rhs);
const FILETIME operator+(const FILETIME &lhs, const FILETIME &rhs);
const FILETIME operator+(const FILETIME &lhs, const __int64 &rhs);
const FILETIME operator+(const __int64 &lhs, const FILETIME &rhs);

std::wstring loadFileAsString(const std::wstring &fileName);
std::vector<std::wstring> loadStringsFromFile(const std::wstring &fileName);

std::string convertUnicode(const std::wstring &uni);
std::wstring convertUnicode(const std::string &uni);
std::wstring GetShortPathNameStr(std::wstring longPath);

std::wstring& expandEnvironmentString(std::wstring& toExpand);

FILETIME UnixTimeToFileTime(const DWORD &t);

#endif //_UTILITY_H_INCLUDED
