//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#include <cstddef>
#include <cstdio>
#include "Windows.h"

namespace wait
{
    inline void PrintErrorMessage()
    {
        std::puts("Usage: pevFind WAIT [Number Of Milliseconds]");
    }

    inline int main(int argc, wchar_t **args)
    {
        if (argc != 2)
        {
            PrintErrorMessage();
            return -1;
        }
        DWORD milliseconds = 0;
        int scanResult = swscanf_s(args[1], L"%i", &milliseconds);
        if (scanResult == 0)
        {
            PrintErrorMessage();
            return -1;
        }
        ::Sleep(milliseconds);
        return 0;
    }
}
