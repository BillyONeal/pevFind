//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// regex.cpp -- Implements some functions for pevFind's
//               vfind regex node.
#include "pch.hpp"
#include <string>
#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include "regex.h"
#include "fpattern.h"
#include "globalOptions.h"
#include "fileData.h"

void stripEscapes(std::wstring& input)
{
	std::remove_if(input.begin(), input.end(), std::bind2nd(std::equal_to<wchar_t>(), L'`'));
}

vFindRegex::vFindRegex(std::wstring patternInput, bool recursive)
{
	if (recursive)
		state = new RvFindRegex();
	else
		state = new NRvFindRegex();
	if (globalOptions::expandRegex && std::find(patternInput.begin(), patternInput.end(), L'\\') != patternInput.end())
	{
		DWORD len = GetLongPathName(patternInput.c_str(), NULL, NULL);
		wchar_t *tempExpandedPath = new wchar_t[len];
		GetLongPathName(patternInput.c_str(), tempExpandedPath, len);
		patternInput = tempExpandedPath;
		delete [] tempExpandedPath;
	}
	std::wstring::const_iterator middle(std::find(patternInput.rbegin(), patternInput.rend(), L'\\').base());
	if (middle == patternInput.begin())
	{
		regex = patternInput;
	} else
	{
		regex = std::wstring(middle, patternInput.end());
		pathRoot = std::wstring(patternInput.begin(), middle);
	}
	if (!fpattern_isvalid(regex.c_str()))
		throw L"There is an error in the syntax of your VFIND regular expression.";
	stripEscapes(pathRoot);
}

vFindRegex::~vFindRegex()
{
	delete state;
}

unsigned __int32 vFindRegex::getPriorityClass() const
{
	return PRIORITY_VFIND_REGEX;
}

BOOL vFindRegex::include(FileData &file) const
{
	return state->include(file, regex, pathRoot);
}

unsigned int vFindRegex::directoryCheck(const std::wstring& directory) const
{
	return state->directoryCheck(directory, pathRoot);
}

bool vFindRegex::isRecursive()
{
	return state->isRecursive();
}

BOOL RvFindRegex::include(FileData &file, const std::wstring& regex, const std::wstring& pathRoot) const
{
	if (pathRoot.size())
	{
		if (!boost::algorithm::istarts_with(file.getFileName(), pathRoot))
			return false;
	}
	return fpattern_matchn(regex.c_str() , file.getFileName().c_str() + (std::find(file.getFileName().rbegin(), file.getFileName().rend(), L'\\').base() - file.getFileName().begin()));
}

BOOL NRvFindRegex::include(FileData &file, const std::wstring& regex, const std::wstring& pathRoot) const
{
	if (!boost::algorithm::iequals(pathRoot,
		boost::make_iterator_range(file.getFileName().begin(),
		std::find(file.getFileName().rbegin(), file.getFileName().rend(), L'\\').base())
		))
		return false;
	return fpattern_matchn(regex.c_str(), file.getFileName().c_str() + (file.getFileName().end() - (std::find(file.getFileName().rbegin(), file.getFileName().rend(), L'\\').base() + 1)) );
}

unsigned int RvFindRegex::directoryCheck(const std::wstring& directory, const std::wstring& pathRoot) const
{
	if (pathRoot.empty())
		return DIRECTORY_DONTCARE;
	size_t dirLength = directory.length();
	if (dirLength > pathRoot.size())
	{
		if (boost::algorithm::istarts_with(directory,pathRoot))
			return DIRECTORY_INCLUDE;
		else
			return DIRECTORY_EXCLUDE;
	}
	if (boost::algorithm::istarts_with(pathRoot,directory))
		return DIRECTORY_INCLUDE;
	else
		return DIRECTORY_EXCLUDE;
}

unsigned int NRvFindRegex::directoryCheck(const std::wstring& directory, const std::wstring& pathRoot) const
{
	if (!boost::algorithm::iequals(pathRoot, directory))
		return DIRECTORY_EXCLUDE;
	return DIRECTORY_INCLUDE;
}

std::wstring vFindRegex::debugTree() const
{
	return std::wstring(L"- VFIND REGEX\r\n   + DIR:   ") + std::wstring(pathRoot) + std::wstring(L"\r\n   + REGEX: ") + std::wstring(regex) + std::wstring(L"\r\n");
}

unsigned int filesRegexPlaceHolder::directoryCheck(const std::wstring& /*directory*/) const
{
	return DIRECTORY_DONTCARE;
};
unsigned __int32 filesRegexPlaceHolder::getPriorityClass() const
{
	return PRIORITY_FAST_FILTER;
};
BOOL filesRegexPlaceHolder::include(FileData &file) const
{
	UNREFERENCED_PARAMETER(file);
	return true;
}
std::wstring filesRegexPlaceHolder::debugTree() const
{
	return L"+ FILES REGEX PLACEHOLDER\r\n";
};

unsigned __int32 perlRegex::getPriorityClass() const
{
	return PRIORITY_PERL_REGEX;
};
BOOL perlRegex::include(FileData &file) const
{
	boost::xpressive::wsmatch results;
	if (boost::xpressive::regex_search(file.getFileName().begin(), file.getFileName().end(), results, regex))
		return true;
	return false;
};
unsigned int perlRegex::directoryCheck(const std::wstring& /*directory*/) const
{
	return DIRECTORY_DONTCARE;
};
std::wstring perlRegex::debugTree() const
{
	return std::wstring(L"- PERL REGEX: ") + regexString + std::wstring(L"\r\n");
};
perlRegex::perlRegex(const std::wstring& inRegex) : regexString(inRegex)
{
	regex = boost::xpressive::wsregex::compile(inRegex.begin(), inRegex.end(), boost::xpressive::regex_constants::icase | boost::xpressive::regex_constants::optimize);
}

void vFindRegex::makeNonRecursive()
{
	delete state;
	state = new NRvFindRegex();
}
