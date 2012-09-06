//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// regImport.cpp -- Implements the "rimport" subprogram.
#include "pch.hpp"
#include <vector>
#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include "regImport.h"
#include "regscriptCompiler.h"
#include "utility.h"

namespace regImport {

void import(const std::wstring& fileName);

int main(int argc, wchar_t* argv[])
{
	if (argc <= 1) throw std::invalid_argument("At least one argument is required to RIMPORT!");
	bool loose = false;
	regscriptCompiler op;
	if (boost::algorithm::iequals(argv[1], L"LOOSE")) loose = true;
	if (loose && argc <= 1) throw std::invalid_argument("At least two arguments are required in LOOSE mode!");
	//Allow multiple files to be imported.
	for(int num = loose ? 2 : 1; num < argc; num++)
	{
		op.parse(loadFileAsString(argv[num]));
	}
#ifdef NDEBUG
	if (loose || op.succeeded())
		op.execute();
#else
	op.printASM();
#endif
	std::wcerr << op.getOutput();
	return 0;
}

} //namespace regImport
