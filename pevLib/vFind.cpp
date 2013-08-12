//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// vFind.cpp -- Implements the main entry point for pevFind.

#include "pch.hpp"
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
        std::puts("# DEBUGGING OUTPUT #");
        std::wprintf(L"Format:\n%s\n\n", globalOptions::displaySpecification.c_str());
        if (globalOptions::debug)
            std::puts("Display debugging output");
        if (globalOptions::fullPath)
            std::puts("Displaying the full path");
        if (globalOptions::summary)
            std::puts("Displaying summary before termination");
        if (globalOptions::noSubDirectories)
            std::puts("Do not recurse into subdirectories");
        std::printf("Encoding: ");
        switch(globalOptions::encoding)
        {
        case globalOptions::ENCODING_TYPE_ACP:
            std::puts("ACP");
            break;
        case globalOptions::ENCODING_TYPE_OEM:
            std::puts("OEM");
            break;
        case globalOptions::ENCODING_TYPE_UTF8:
            std::puts("UTF-8");
            break;
        case globalOptions::ENCODING_TYPE_UTF16:
            std::puts("UTF-16");
            break;
        }
        std::printf("Limiting to %u lines\n", globalOptions::lineLimit);
        if (globalOptions::timeout)
            std::printf("Limiting to %u ms of execution time\n", globalOptions::timeout);
        std::fputws(L"\nInternal processing tree:\n", stdout);
        std::wstring debugTreeResult(globalOptions::logicalTree->debugTree());
        std::fputws(debugTreeResult.c_str(), stdout);
    }
    if (globalOptions::regularExpressions.empty())
    {
        std::fprintf(stderr, "Search operations must specify at least one regex.");
        return 3;
    }
    globalOptions::logicalTree->reorderTree();
    if (globalOptions::debug)
    {
        std::wstring debugTree(globalOptions::logicalTree->debugTree());
        std::wprintf(L"\nInternal processing tree after reordering:\n%s\n# END DEBUGGING OUTPUT #\n\n", debugTree.c_str());
        system("pause");
    }
    //If a timeout is set, start the watch thread to terminate this one if need be.
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
