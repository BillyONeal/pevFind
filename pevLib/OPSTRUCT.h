//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// Opstruct.h -- Defines structures which control analysis
// flow inside pevFind's tree
#ifndef _OPSTRUCT_H_INCLUDED
#define _OPSTRUCT_H_INCLUDED
#include <vector>
#include <boost/shared_ptr.hpp>
#include "criterion.h"

class operation : public criterion
{
protected:
	boost::shared_ptr<criterion> operandA;
	boost::shared_ptr<criterion> operandB;
	std::wstring debugTreeInternal(const std::wstring& curOp) const;
public:
	virtual void reorderTree();
	unsigned __int32 getPriorityClass() const;
	operation(boost::shared_ptr<criterion> a, boost::shared_ptr<criterion> b);
	void makeNonRecursive();
};

class andAndClass : public operation
{
public:
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory) const;
	andAndClass(boost::shared_ptr<criterion> a, boost::shared_ptr<criterion> b);
	std::wstring debugTree() const;
};
class orAndClass : public operation
{
public:
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory) const;
	orAndClass(boost::shared_ptr<criterion> a, boost::shared_ptr<criterion> b);
	std::wstring debugTree() const;
};
class xorAndClass : public operation
{
public:
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory) const;
	xorAndClass(boost::shared_ptr<criterion> a, boost::shared_ptr<criterion> b);
	std::wstring debugTree() const;
};
class notAndClass : public criterion
{
	boost::shared_ptr<criterion> operand;
public:
	virtual void reorderTree();
	unsigned __int32 getPriorityClass() const;
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory) const;
	notAndClass(boost::shared_ptr<criterion> a);
	std::wstring debugTree() const;
	void makeNonRecursive();
};
class bracketClass : public criterion
{
	std::vector<boost::shared_ptr<criterion> > expr;
public:
	virtual void reorderTree();
	unsigned __int32 getPriorityClass() const;
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory);
	bracketClass(std::vector<boost::shared_ptr<criterion> > exprA);
	std::wstring debugTree() const;
	void makeNonRecursive();
};

class ifClass : public criterion
{
	boost::shared_ptr<criterion> condVal;
	boost::shared_ptr<criterion> tVal;
	boost::shared_ptr<criterion> fVal;
public:
	virtual void reorderTree();
	unsigned __int32 getPriorityClass() const;
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory) const;
	ifClass(boost::shared_ptr<criterion> condition,boost::shared_ptr<criterion> valueIfTrue,boost::shared_ptr<criterion> valueIfFalse = boost::shared_ptr<criterion>((criterion *)NULL));
	std::wstring debugTree() const;
	void makeNonRecursive();
};

#endif
