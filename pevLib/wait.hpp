//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#include <iostream>
#include <sstream>
#include "Windows.h"

namespace wait
{
	inline void PrintErrorMessage()
	{
		std::cout << "Usage: pevFind WAIT [Number Of Milliseconds]";
	}

	inline int main(int argc, wchar_t **args)
	{
		if (argc != 2)
		{
			PrintErrorMessage();
			return -1;
		}
		std::size_t milliseconds = 0;
		std::wistringstream numberConverter(args[1]);
		if (!(numberConverter >> milliseconds))
		{
			PrintErrorMessage();
			return -1;
		}
		::Sleep(static_cast<DWORD>(milliseconds));
		return 0;
	}
}
