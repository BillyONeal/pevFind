//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// pevFind.cpp -- Defines the main entry point for the executable. Bootstraps
// associated subprograms.

#include <iostream>
#include <fstream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/scoped_array.hpp>
#include "processManager.h"
#include "utility.h"

//Subprogram definitions
#include "clsidCompressor.h"
#include "procListers.h"
#include "exec.h"
#include "vFind.h"
#include "link.h"
#include "volumeEnumerate.h"
#include "uZip.h"
#include "times.h"
#include "regImport.h"
#include "rexport.h"
#include "moveex.h"
#include "serviceControl.h"
#include "dosdev.h"
#include "linkResolve.h"

int __cdecl wmain(int argc, wchar_t* argv[])
{
	using namespace boost::algorithm;
	int subArgC;
	boost::scoped_array<wchar_t *> subArgV;
	try {
	if (argc < 2) throw std::runtime_error("Invalid number of arguments."); //We always require at least one option.
	
	if (iequals(argv[1], L"vfind")) //vFind is the default, and therefore we don't need to bother reallocating
		return vFind::main(); //It's argument pointers and such.
	
	subArgC = argc - 1; //Remove subprogram option, and regenerate our list, with the subprogram selected removed.
	subArgV.reset(new wchar_t * [subArgC]);
	subArgV[0] = argv[0];
	memcpy(subArgV.get() + 1, argv + 2, (subArgC - 1) * sizeof(wchar_t *));
	
	//When we use a single iterator_range for all the following comparisons, strlen is called much less often.
	boost::iterator_range<wchar_t *> firstArgument(argv[1], argv[1] + wcslen(argv[1]));
	
	if      (iequals(firstArgument, L"CLSID"))
		return clsidCompressor::main(subArgC, subArgV.get());
	else if (iequals(firstArgument, L"EXEC"))
		return exec::main();
	else if (iequals(firstArgument, L"PLIST"))
		return plist::main(subArgC, subArgV.get());
	else if (iequals(firstArgument, L"CLIST"))
		return clist::main();
	else if (iequals(firstArgument, L"LINK"))
		return link::main(subArgC, subArgV.get());
	else if (iequals(firstArgument, L"VOLUME"))
		return volumeEnumerate::main();
	else if (iequals(firstArgument, L"UZIP"))
		return uZip::main(subArgC, subArgV.get());
	else if (iequals(firstArgument, L"TIME"))
		return times::main(subArgC, subArgV.get());
	else if (iequals(firstArgument, L"RIMPORT"))
		return regImport::main(subArgC, subArgV.get());
	else if (iequals(firstArgument, L"MOVEEX"))
		return moveex::main(subArgC, subArgV.get());
	else if (iequals(firstArgument, L"SC"))
		return serviceControl::main(subArgC, subArgV.get());
	else if (iequals(firstArgument, L"DDEV"))
		return dosdev::main(subArgC, subArgV.get());
	else if (iequals(firstArgument, L"LINKRESOLVE"))
		return linkResolve::main(subArgC, subArgV.get());
	else if (iequals(firstArgument, L"REXPORT"))
		return rexport::main(subArgC, subArgV.get());
	return vFind::main();
	}
#ifdef NDEBUG
	catch(...)
	{
std::cerr << 
"                         _____ _           _ \n"
"        _ __   _____   _|  ___(_)_ __   __| |\n"
"       | '_ \\ / _ \\ \\ / / |_  | | '_ \\ / _` |\n"
"       | |_) |  __/\\ V /|  _| | | | | | (_| |\n"
"       | .__/ \\___| \\_/ |_|   |_|_| |_|\\__,_|\n"
"       |_|    by Billy Robert O'Neal III\n"
"                      Version 1.5.5\n"
"  Distributed under the Boost Software License, Version 1.0.\n"
"         http://www.boost.org/LICENSE_1_0.txt\n"
"pevFind contains some code from Info-ZIP, used with permission.\n"
"  In accordance with Info-ZIP's License, it can be found at\n"
"           http://billy-oneal.com/infozip.txt\n"
"            Filename regular expressions library is\n"
" Copyright (C)1997-1998 by David R. Tribble, all rights reserved.\n\n";
	}
#else
	catch (std::exception& except)
	{
		std::cerr << except.what();
	}
#endif
}
