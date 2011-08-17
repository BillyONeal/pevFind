//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// times.cpp -- Implements the time subprogram.
#include "pch.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <boost/algorithm/string/predicate.hpp>
#include "times.h"
#include "logger.h"
#include "utility.h"

int times::main(int argc, wchar_t* argv[])
{
	SYSTEMTIME timestruct;
	if ((argc >= 2) && boost::algorithm::iequals(argv[1],L"utc"))
		GetSystemTime(&timestruct);
	else
		GetLocalTime(&timestruct);
	FILETIME fTimeStruct;
	SystemTimeToFileTime(&timestruct, &fTimeStruct);
	logger << getDateAsString(fTimeStruct) << L"\r\n";
	return 0;
}
