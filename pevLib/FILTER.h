//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// Filter.h -- defines filters which are used to show or
//               hide a particular file.
#pragma once
#include <vector>
#include <string>
#include "criterion.h"

class sizeFilter : public criterion
{
protected:
	unsigned __int64 size;
	std::wstring debugTreeInternal(const std::wstring& type) const;
public:
	unsigned __int32 getPriorityClass() const;
	sizeFilter(unsigned __int64 createdSize);
};

struct gtSizeFilter : sizeFilter
{
	gtSizeFilter(unsigned __int64 createdSize);
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};

struct ltSizeFilter : sizeFilter
{
	ltSizeFilter(unsigned __int64 createdSize);
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};

struct notSizeFilter : sizeFilter
{
	notSizeFilter(unsigned __int64 createdSize);
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};

struct isSizeFilter : sizeFilter
{
	isSizeFilter(unsigned __int64 createdSize);
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};

class dateFilter : public criterion
{
protected:
	FILETIME date;
public:
	virtual unsigned __int32 getPriorityClass() const;
	dateFilter(const FILETIME &inDate);
	
};

struct headerLDate : dateFilter
{
	unsigned __int32 getPriorityClass() const;
	headerLDate(const FILETIME &inDate) : dateFilter(inDate) {};
	std::wstring debugTree() const;
	BOOL include(FileData &file) const;
};

struct headerGDate : dateFilter
{
	unsigned __int32 getPriorityClass() const;
	headerGDate(const FILETIME &inDate) : dateFilter(inDate) {};
	std::wstring debugTree() const;
	BOOL include(FileData &file) const;
};

class accessLDate : public dateFilter
{
public:
	accessLDate(const FILETIME &inDate);
	std::wstring debugTree() const;
	BOOL include(FileData &file) const;
};

class accessGDate : public dateFilter
{
public:
	accessGDate(const FILETIME &inDate);
	std::wstring debugTree() const;
	BOOL include(FileData &file) const;
};

class modifiedLDate : public dateFilter
{
public:
	modifiedLDate(const FILETIME &inDate);
	std::wstring debugTree() const;
	BOOL include(FileData &file) const;
};

class modifiedGDate : public dateFilter
{
public:
	modifiedGDate(const FILETIME &inDate);
	std::wstring debugTree() const;
	BOOL include(FileData &file) const;
};

class createdLDate : public dateFilter
{
public:
	createdLDate(const FILETIME &inDate);
	std::wstring debugTree() const;
	BOOL include(FileData &file) const;
};

class createdGDate : public dateFilter
{
public:
	createdGDate(const FILETIME &inDate);
	std::wstring debugTree() const;
	BOOL include(FileData &file) const;
};

struct fastFilter : public criterion
{
	unsigned __int32 getPriorityClass() const;
};

struct isArchive : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isCompressed : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isDirectory : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isFile : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isReparsePoint : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isSFCProtected : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isHidden : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isReadOnly : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isSystem : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isVolumeLabel : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isWritable : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isTemp : public fastFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct sigIsValid : public criterion
{
	unsigned __int32 getPriorityClass() const;
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct peFilter : public criterion
{
	unsigned __int32 getPriorityClass() const;
};
struct hasSig : public peFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isPEFile : public peFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isPEPlusFile : public peFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isNEFile : public peFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isLEFile : public peFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isMZFile : public peFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct is2ExecFile : public peFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct timestampValid : public peFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct checkSumValid : public peFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};
struct isDLLFile : public peFilter
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
};

struct hash : public criterion
{
	unsigned __int32 getPriorityClass() const;
};

class md5Match : public hash
{
	std::wstring md5Val;
public:
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
	md5Match(std::wstring md5Value);
};
class sha1Match : public hash
{
	std::wstring sha1Val;
public:
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
	sha1Match(std::wstring sha1Value);
};
class hashList : public hash
{
protected:
	std::vector<std::wstring> values;
	std::wstring listValues() const;
public:
	hashList(std::vector<std::wstring> hashes);
};
struct md5List : public hashList
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
	md5List(std::vector<std::wstring> md5s);
};
struct sha1List : public hashList
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
	sha1List(std::vector<std::wstring> sha1s);
};
struct md5EList : public hashList
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
	md5EList(std::vector<std::wstring> md5s);
};
struct sha1EList : public hashList
{
	BOOL include(FileData &file) const;
	std::wstring debugTree() const;
	sha1EList(std::vector<std::wstring> sha1s);
};
class skipper : public criterion
{
	std::wstring _toSkip;
public:
	unsigned __int32 getPriorityClass() const;
	BOOL include(FileData & /*file*/) const;
	std::wstring debugTree() const;
	virtual unsigned int directoryCheck(const std::wstring& directory);
	skipper(const std::wstring& toSkip);
};
