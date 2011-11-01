//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#include <windows.h>
#include <vector>
#include <string>

namespace vFind {

class FileData;

struct IInputProvider
{
	virtual ~IInputProvider() { }
	virtual FileData Next() = 0;
};

class FileInput : public IInputProvider
{
	std::wstring root;
	HANDLE hSearch;
public:
	FileInput(const std::wstring& rootPath) : root(rootPath), hSearch(0) {}
	~FileInput();
	virtual FileData Next();
};

class ProcessInput : public IInputProvider
{
	std::vector<std::wstring> processes;
	std::vector<std::wstring>::iterator current;
public:
	ProcessInput();
	virtual FileData Next();
};

struct IFilter
{
	virtual ~IFilter() {};
	virtual bool Apply(const FileData&) = 0;
	virtual bool End() {}
	virtual bool HasResults() const { return false; }
	virtual std::vector<FileData> Results();
};

class PevTreeFilter : public IFilter
{
	virtual bool Apply(const FileData&);
	virtual std::vector<FileData> Results();
};

class StatisticsFilter : public IFilter
{
	virtual bool Apply(const FileData&);
	virtual std::vector<FileData> Results();
};

}
