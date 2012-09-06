//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// processScanner.cpp -- Implements process kill mode
#include "pch.hpp"
#include <vector>
#include <list>
#include "processManager.h"
#include "processScanner.h"
#include "criterion.h"
#include "globalOptions.h"
#include "fileData.h"

namespace scanners {

	void processScanner::scan()
	{
		if (globalOptions::noSubDirectories)
			globalOptions::logicalTree->makeNonRecursive();
		std::list<FileData> results;
		processManager mgr;
		std::vector<process> source(mgr.enumerate());
		for (std::vector<process>::iterator it = source.begin(); it != source.end(); it++)
		{
			try 
			{
				FileData theRecord(it->executablePath());
				if (globalOptions::logicalTree->include(theRecord))
				{
					it->kill();
				}
			} catch (...) {};
		}
	}

}