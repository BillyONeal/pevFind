//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// volumeEnumerate.cpp -- Implements the volume enumeration
// sub program.

#include "pch.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "volumeEnumerate.h"
#include "logger.h"

int volumeEnumerate::main()
{
    DWORD logicalDrives = GetLogicalDrives();
    DWORD bitMask = 1;
    wchar_t driveTemp[] = L"A:\\";
    for (register size_t idx = 0; idx < 26; idx++)
    {
        if (logicalDrives & bitMask)
        {
            driveTemp[0] = (wchar_t) idx + L'A';
            if (GetDriveType(driveTemp) == DRIVE_FIXED)
            {
                logger << driveTemp << L"\r\n";
            }
        }
        bitMask <<= 1;
    }
    return 0;
}
