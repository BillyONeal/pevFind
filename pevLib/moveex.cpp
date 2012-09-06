//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// moveEx.cpp -- Implements the move on reboot sub program.

#include "pch.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "moveex.h"
#include "logger.h"
#include "timeoutThread.h"

namespace moveex {

int main(int argc, wchar_t* argv[])
{
	CreateThread(NULL,50,&timeoutThread,reinterpret_cast<void *>(static_cast<DWORD>(3000)),NULL,NULL);
	switch (argc)
	{
	case 2:
		MoveFileEx(argv[1], NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		return 0;
	case 3:
		MoveFileEx(argv[1], argv[2], MOVEFILE_DELAY_UNTIL_REBOOT);
		return 0;
	default:
		throw std::runtime_error("Invalid Syntax.");
	}
}

} // namespace moveex
