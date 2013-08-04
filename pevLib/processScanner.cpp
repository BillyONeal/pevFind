//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// processScanner.cpp -- Implements process kill mode
#include "pch.hpp"
#include <vector>
#include "processScanner.h"
#include "criterion.h"
#include "globalOptions.h"
#include "fileData.h"
#include "../LogCommon/Win32Exception.hpp"
#include "../LogCommon/Process.hpp"

namespace scanners {

    void processScanner::scan()
    {
        if (globalOptions::noSubDirectories)
		{
            globalOptions::logicalTree->makeNonRecursive();
		}

		Instalog::SystemFacades::ProcessEnumerator processes;
		for (auto process : processes)
        {
            try 
            {
                FileData theRecord(process.GetExecutablePath());
                if (globalOptions::logicalTree->include(theRecord))
                {
                    process.Terminate();
                }
            }
			catch (Instalog::SystemFacades::Win32Exception const&)
			{}
        }
    }
}
