//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// globalOptions::cpp -- Gives the global data structure an
// implementation file to use.
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "globalOptions.h"
#include "regex.h"

std::vector<boost::shared_ptr<regexClass> > globalOptions::regularExpressions;
boost::shared_ptr<criterion>  globalOptions::logicalTree;
bool globalOptions::showall = false;
bool globalOptions::debug = false;
bool globalOptions::fullPath = false;
bool globalOptions::summary = false;
bool globalOptions::noSubDirectories = false;
std::wstring globalOptions::displaySpecification = L"#f";
globalOptions::sorts globalOptions::sortMethod[6] = {globalOptions::NONE, globalOptions::NONE, globalOptions::NONE, globalOptions::NONE, globalOptions::NONE, globalOptions::NONE} ;
globalOptions::encodings globalOptions::encoding = globalOptions::ENCODING_TYPE_ACP;
unsigned __int64 globalOptions::lineLimit = static_cast <unsigned int> (-1);
unsigned __int32 globalOptions::timeout = 150000;
bool globalOptions::cancel = false;
unsigned __int64 globalOptions::totalEntries = 0;
unsigned __int64 globalOptions::visibleEntries = 0;
unsigned __int64 globalOptions::totalSize = 0;
unsigned __int64 globalOptions::visibleFiles = 0;
unsigned __int64 globalOptions::visibleDirs = 0;
unsigned __int64 globalOptions::blocks = 0;
std::vector<std::wstring> globalOptions::fileList;
bool globalOptions::expandRegex = false;
bool globalOptions::disable64Redirector = true;
std::wstring globalOptions::zipFileName;
bool globalOptions::killProc = false;