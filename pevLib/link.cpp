//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// moveEx.cpp -- Implements the move on reboot sub program.

#include "pch.hpp"
#include <stdexcept>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "link.h"

namespace link {

int main(int argc, wchar_t* argv[])
{
	if (argc < 3)
		throw std::runtime_error("Invalid syntax!");
	CreateHardLink(argv[2], argv[1], NULL);
	return 0;
}

};
