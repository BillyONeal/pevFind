//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <string>
#include <algorithm>
#include <Windows.h>

namespace pevFind { namespace tests
{
    inline std::wstring GetTestDllPath()
    {
        std::wstring (*ptr)() = GetTestDllPath; 
        wchar_t const * ptrVoid = reinterpret_cast<wchar_t const*>(ptr);
        HMODULE hMod;
        ::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, ptrVoid, &hMod);
        std::wstring result;
        result.resize(MAX_PATH);
        result.resize(::GetModuleFileNameW(hMod, &result[0], MAX_PATH));
        return result;
    }

    inline std::wstring GetTestDllDir()
    {
        std::wstring result(GetTestDllPath());
        result.replace(std::find(result.crbegin(), result.crend(), L'\\').base(), result.cend(), L"TestData\\");
        return result;
    }

    inline std::wstring GetTestDllFilePath(std::wstring const& file)
    {
        return GetTestDllDir().append(file);
    }

    inline std::wstring Str(wchar_t const* val)
    {
        return val;
    }
} }
