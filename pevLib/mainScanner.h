#ifndef _MAINSCANNER_H_FILE_INCLUDED
#define _MAINSCANNER_H_FILE_INCLUDED
//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// mainScanner.h -- The primary scanner, this scanner is
// the one used by default. It recurses into subdirectories
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "regex.h"
class FileData;

namespace scanners
{
	std::wstring getRegexesCommonRoot(std::vector<boost::shared_ptr<regexClass> >&);
	void printSummary();
	class recursiveScanner
	{	
	public:
		void scan();
	};
};
#endif
