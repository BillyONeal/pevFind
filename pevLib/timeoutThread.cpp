//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// timeoutThread.cpp -- Implements the timeout thread function.

#include "pch.hpp"
#include <cstdlib>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "timeoutThread.h"

DWORD WINAPI timeoutThread(LPVOID timeout)
{
	Sleep(reinterpret_cast<DWORD>(timeout)); //Sleep the length of the user specified timeout
	// NOTE: If the main thread finishes before this Sleep() does, then it
	// will terminate the process, and this thread will be killed.
	std::exit(2);
}