#ifndef _GLOBAL_OPTIONS_H_INCLUDED
#define _GLOBAL_OPTIONS_H_INCLUDED
//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// globalOptions::h -- Defines the global process block which
// contains the settings for a run of pevFind shared by all
// components.
#include <string>
#include <vector>

class regexClass;
class criterion;
class subProgramClass;

class globalOptions
{
public:
	static std::vector<std::tr1::shared_ptr<regexClass> > regularExpressions;
	static std::tr1::shared_ptr<criterion> logicalTree;
	static bool showall;
	static bool debug;
	static bool fullPath;
	static bool summary;
	static bool noSubDirectories;
	static std::wstring displaySpecification;
	static unsigned __int64 totalEntries;
	static unsigned __int64 visibleEntries;
	static unsigned __int64 totalSize;
	static unsigned __int64 visibleFiles;
	static unsigned __int64 visibleDirs;
	static unsigned __int64 blocks;
	enum sorts
	{
		NONE = 0,
		SIZE,
		NAME,
		DSIZE,
		DNAME,
		CDATE,
		MDATE,
		ADATE,
		DCDATE,
		DMDATE,
		DADATE,
		HDATE,
		DHDATE,
		INAME,
		DINAME
	};
	static sorts sortMethod[6];
	enum encodings
	{
		ENCODING_TYPE_UTF8,
		ENCODING_TYPE_UTF16,
		ENCODING_TYPE_ACP,
		ENCODING_TYPE_OEM
	};
	static encodings encoding;
	static unsigned __int64 lineLimit;
	static unsigned __int32 timeout;
	static bool cancel;
	static void addSort( sorts toAdd )
	{
		static sorts *target = sortMethod;
		*target = toAdd;
		target++;
		if (target >= sortMethod + 5)
		{
			throw L"You can't have more than 5 sort methods... and why would you want to?!?";
		}
	}
	static std::vector<std::wstring> fileList;
	static bool expandRegex;
	static bool disable64Redirector;
	static std::wstring zipFileName;
	static bool killProc;
};
#endif //_GLOBAL_OPTIONS_H_INCLUDED
