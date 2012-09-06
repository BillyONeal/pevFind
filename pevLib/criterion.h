//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// criterion.h -- The root class from which all criteria inherit.
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class FileData;

#define PRIORITY_FAST_FILTER 1
#define PRIORITY_SLOW_FILTER 2
#define PRIORITY_VFIND_REGEX 3
#define PRIORITY_PERL_REGEX 3
#define PRIORITY_HASH_CHECK 6
#define PRIORITY_PE_DATA 4
#define PRIORITY_SIGCHECK 7
#define PRIORITY_HEX_SEARCH 5

#define DIRECTORY_DONTCARE 2
#define DIRECTORY_INCLUDE 1 
#define DIRECTORY_EXCLUDE 0

class criterion
{
public:
	virtual unsigned __int32 getPriorityClass() const = 0;
	virtual BOOL include(FileData &file) const = 0;
	virtual void reorderTree() {};
	virtual unsigned int directoryCheck(const std::wstring& /*directory*/) { return DIRECTORY_DONTCARE; };
	virtual std::wstring debugTree() const = 0;
	virtual ~criterion() {}; //Virtual destructor DO NOT REMOVE!
	virtual void makeNonRecursive() {};
};

//Functor which converts pointers to criteria to priority classes
static struct _getPriorityFunctor : std::unary_function<const criterion*, __int32>
{
	__int32 operator()(const std::shared_ptr<criterion> crit)
	{
		return crit->getPriorityClass();
	};
} getPriorityFunctor;

static struct _criterionByPriorityClass : std::binary_function<const std::shared_ptr<criterion>, std::shared_ptr<criterion>, bool>
{
	bool operator()(const std::shared_ptr<criterion> a, const std::shared_ptr<criterion> b)
	{
		return a->getPriorityClass() < b->getPriorityClass();
	}
} criterionByPriorityClass;
