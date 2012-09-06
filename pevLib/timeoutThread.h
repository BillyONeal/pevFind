#ifndef _TIMEOUT_THREAD_H_INCLUDED
#define _TIMEOUT_THREAD_H_INCLUDED
//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// timeoutThread.h -- Defines the timeout thread function.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
DWORD WINAPI timeoutThread(LPVOID /*nothing*/);
#endif _TIMEOUT_THREAD_H_INCLUDED
