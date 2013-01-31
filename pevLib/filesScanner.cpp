//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// filesScanner.cpp -- Implements the scanner used if one specifies
// the  --files directive.
#include "pch.hpp"
#include <list>
#include <vector>
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include "utility.h"
#include "filesScanner.h"
#include "globalOptions.h"
#include "fileData.h"
#include "criterion.h"
#include "zipIt.h"

namespace scanners {

void filesScanner::scan()
{
    std::list<FileData> results;
    //Loop through files the user has entered
    for( std::vector<std::wstring>::iterator it = globalOptions::fileList.begin(); it != globalOptions::fileList.end(); it++ )
    {
        disable64.disableFS(); //Shutdown WOW64.
        if (!PathFileExists(it->c_str())) //If they do not exist, skip to the next file
            continue;
        disable64.enableFS(); //Restart WOW64.
        FileData curFileStructed(*it); //Create a fileData object to pass through PEV's tree
        if (!globalOptions::logicalTree->include(curFileStructed)) //Check if the file is valid in the tree
            continue; //Skip to the next file otherwise
        results.push_back(curFileStructed); //File exists, add it to results
    }
    if (globalOptions::sortMethod[0]) //Print results
        results.sort();
    for(std::list<FileData>::iterator it = results.begin(); it != results.end(); it++)
    {
        it->write();
    }
    if (!globalOptions::zipFileName.empty())
        zipIt(globalOptions::zipFileName, results);
    printSummary();
}

}; //Namespace scanners