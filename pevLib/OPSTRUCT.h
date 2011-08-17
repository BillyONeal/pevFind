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
#include "criterion.h"

class operation : public criterion
{
protected:
	std::tr1::shared_ptr<criterion> operandA;
	std::tr1::shared_ptr<criterion> operandB;
	std::wstring debugTreeInternal(const std::wstring& curOp) const;
public:
	virtual void reorderTree();
	unsigned __int32 getPriorityClass() const;
	operation(std::tr1::shared_ptr<criterion> a, std::tr1::shared_ptr<criterion> b);
	void makeNonRecursive();
};

class andAndClass : public operation
{
public:
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory) const;
	andAndClass(std::tr1::shared_ptr<criterion> a, std::tr1::shared_ptr<criterion> b);
	std::wstring debugTree() const;
};
class orAndClass : public operation
{
public:
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory) const;
	orAndClass(std::tr1::shared_ptr<criterion> a, std::tr1::shared_ptr<criterion> b);
	std::wstring debugTree() const;
};
class xorAndClass : public operation
{
public:
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory) const;
	xorAndClass(std::tr1::shared_ptr<criterion> a, std::tr1::shared_ptr<criterion> b);
	std::wstring debugTree() const;
};
class notAndClass : public criterion
{
	std::tr1::shared_ptr<criterion> operand;
public:
	virtual void reorderTree();
	unsigned __int32 getPriorityClass() const;
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory) const;
	notAndClass(std::tr1::shared_ptr<criterion> a);
	std::wstring debugTree() const;
	void makeNonRecursive();
};
class bracketClass : public criterion
{
	std::vector<std::tr1::shared_ptr<criterion> > expr;
public:
	virtual void reorderTree();
	unsigned __int32 getPriorityClass() const;
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory);
	bracketClass(std::vector<std::tr1::shared_ptr<criterion> > exprA);
	std::wstring debugTree() const;
	void makeNonRecursive();
};

class ifClass : public criterion
{
	std::tr1::shared_ptr<criterion> condVal;
	std::tr1::shared_ptr<criterion> tVal;
	std::tr1::shared_ptr<criterion> fVal;
public:
	virtual void reorderTree();
	unsigned __int32 getPriorityClass() const;
	BOOL include(FileData &file) const;
	virtual unsigned int directoryCheck(const std::wstring& directory) const;
	ifClass(std::tr1::shared_ptr<criterion> condition,std::tr1::shared_ptr<criterion> valueIfTrue,std::tr1::shared_ptr<criterion> valueIfFalse = std::tr1::shared_ptr<criterion>((criterion *)NULL));
	std::wstring debugTree() const;
	void makeNonRecursive();
};

#endif
