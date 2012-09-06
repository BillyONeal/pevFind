//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// zipIt.h -- Zips results of scan
#pragma once
#include <string>
#include <list>
class FileData;

void zipIt(const std::wstring& fileTarget,const std::list<FileData>& files);
