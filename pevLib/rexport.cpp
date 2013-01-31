//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// rexport.cpp - Implements the registry export sub program.

#include "pch.hpp"
#include <iostream>
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "rexport.h"
#include "logger.h"

namespace rexport
{
int main(int argc, wchar_t* argv[])
{
    if (argc != 3)
        throw std::invalid_argument("2 arguments required for REXPORT!");
    std::wstring key(argv[1]);
    logger.update(argv[2]);
    logger << L"Windows Registry Editor Version 5.00\r\n"
           << L"\r\n"
           << L";Generator: pevFind by Billy O'Neal III\r\n"
           << L";Start in: " << key << L"\r\n"
           << L";;;\r\n\r\n";

    return 0;
}
};
