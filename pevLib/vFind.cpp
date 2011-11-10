//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// vFind.cpp -- Implements the main entry point for pevFind.

#include "pch.hpp"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "timeoutThread.h"
#include "utility.h"
#include "mainScanner.h"
#include "filesScanner.h"
#include "processScanner.h"
#include "consoleParser.h"
#include "globalOptions.h"
#include "criterion.h"

namespace vFind {

int main()
{
	consoleParser parseInstance;
	globalOptions::logicalTree = parseInstance.parseCmdLine(GetCommandLine());
	if (globalOptions::debug)
	{
		std::wcout << L"# DEBUGGING OUTPUT #\n";
		std::wcout << L"Format:\n" << globalOptions::displaySpecification << L"\n\n";
		if (globalOptions::debug)
			std::wcout << L"Display debugging output\n";
		if (globalOptions::fullPath)
			std::wcout << L"Displaying the full path\n";
		if (globalOptions::summary)
			std::wcout << L"Displaying summary before termination\n";
		if (globalOptions::noSubDirectories)
			std::wcout << L"Do not recurse into subdirectories\n";
		std::wcout << L"Encoding: ";
		switch(globalOptions::encoding)
		{
		case globalOptions::ENCODING_TYPE_ACP:
			std::wcout << L"ACP";
			break;
		case globalOptions::ENCODING_TYPE_OEM:
			std::wcout << L"OEM";
			break;
		case globalOptions::ENCODING_TYPE_UTF8:
			std::wcout << L"UTF-8";
			break;
		case globalOptions::ENCODING_TYPE_UTF16:
			std::wcout << L"UTF-16";
			break;
		}
		std::wcout << L"\n";
		std::wcout << L"Limiting to " << boost::lexical_cast<std::wstring>(globalOptions::lineLimit) << L" lines\n";
		if (globalOptions::timeout)
			std::wcout << L"Limiting to " << boost::lexical_cast<std::wstring>(globalOptions::timeout) << L" ms of execution time\n";
		std::wcout << L"\nInternal processing tree:\n";
		std::wcout << globalOptions::logicalTree->debugTree();
	}
	if (globalOptions::regularExpressions.empty())
	{
		#ifndef NDEBUG
		std::cerr << "Search operations must specify at least one regex.";
		#endif
		return 3;
	}
	globalOptions::logicalTree->reorderTree();
	if (globalOptions::debug)
	{
		std::wcout << L"\nInternal processing tree after reordering:\n";
		std::wcout << globalOptions::logicalTree->debugTree();
		std::wcout << L"\n# END DEBUGGING OUTPUT #\n\n";
		system("pause");
	}
	//If a timeout is set, start the watchthread to terminate this one if need be.
	if (globalOptions::timeout)
		CreateThread(NULL,50,&timeoutThread,reinterpret_cast<LPVOID>(globalOptions::timeout),NULL,NULL);
	if (!globalOptions::fileList.empty())
		scanners::filesScanner().scan();
	else if (globalOptions::killProc)
		scanners::processScanner().scan();
	else
		scanners::recursiveScanner().scan();
#ifndef NDEBUG
	system("pause");
#endif
	if (globalOptions::cancel)
		return 1;
	if (globalOptions::totalEntries)
		return 0;
	else 
		return 4;
}

} //namespace vFind
