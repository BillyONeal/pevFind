#ifndef _REGEX_H_INCLUDED
#define _REGEX_H_INCLUDED
//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// pevFind's tree node for vFind's regex library, as well
// as the placeholder used by the --files switch
#include "criterion.h"
#include <string>
#include <boost/xpressive/xpressive_dynamic.hpp>

class regexClass : public criterion
{
protected:
	std::wstring pathRoot;
public:
	const std::wstring& getPathRoot() { return pathRoot; }
};

/******************************
 * Type code -> State/Strategy
 ******************************/
struct vFindRegexType
{
	virtual BOOL include(FileData &file, const std::wstring& regex, const std::wstring& pathRoot) const = 0;
	virtual unsigned int directoryCheck(const std::wstring& directory, const std::wstring& pathRoot) const = 0;
	virtual bool isRecursive() const = 0;
	virtual ~vFindRegexType() {};
};

struct RvFindRegex : public vFindRegexType
{
	BOOL include(FileData &file, const std::wstring& regex, const std::wstring& pathRoot) const;
	unsigned int directoryCheck(const std::wstring& directory, const std::wstring& pathRoot) const;
	bool isRecursive() const { return true; };
};

struct NRvFindRegex : public vFindRegexType
{
	BOOL include(FileData &file, const std::wstring& regex, const std::wstring& pathRoot) const;
	unsigned int directoryCheck(const std::wstring& directory, const std::wstring& pathRoot) const;
	bool isRecursive() const { return false; };
};

class vFindRegex : public regexClass
{
	vFindRegexType *state;
	std::wstring regex;
public:
	unsigned __int32 getPriorityClass() const;
	BOOL include(FileData &file) const;
	unsigned int directoryCheck(const std::wstring& directory) const;
	bool isRecursive();
	vFindRegex(std::wstring patternInput, bool recursive = true);
	~vFindRegex();
	std::wstring debugTree() const;
	void makeNonRecursive();
};

class filesRegexPlaceHolder : public regexClass
{
public:
	unsigned int directoryCheck(const std::wstring&) const;
	unsigned __int32 getPriorityClass() const;
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};

class perlRegex : public regexClass
{
private:
	std::wstring regexString;
	boost::xpressive::basic_regex< std::wstring::const_iterator > regex;
public:
	unsigned __int32 getPriorityClass() const;
	BOOL include(FileData &file) const;
	unsigned int directoryCheck(const std::wstring& /*directory*/) const;
	std::wstring debugTree() const;
	perlRegex(const std::wstring& inRegex);
};

#endif // _REGEX_H_INCLUDED 
