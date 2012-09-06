#ifndef _LOGGER_H_INCLUDED
#define _LOGGER_H_INCLUDED
//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// logger.h -- Defines the class used to write output to the
// console. Handles encoding conversions.

#include <string>

typedef void * HANDLE;

static class logger_class
{
	static HANDLE stdOut;
	static bool useWriteConsole;
public:
	logger_class();
	~logger_class();
	logger_class& operator<<(const std::wstring& rhs);
	void update(const std::wstring& fileName);
} logger;

#endif //_LOGGER_H_INCLUDED
