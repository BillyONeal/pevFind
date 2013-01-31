//              Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#include <vector>
#include <string>
#include <functional>
#include <windows.h>
#include <boost/optional/optional_fwd.hpp>

class FileData;

namespace vFind {

// An input. Serves as a source of input items.
struct IInputProvider
{
    virtual ~IInputProvider() { }
    // Returns all entries from a given input by calling the indicated functor.
    virtual void Enumerate(const std::tr1::function<void(const FileData&)> nextStage) = 0;
};

class FileInput : public IInputProvider
{
    std::wstring root;
public:
    FileInput(const std::wstring& rootPath);
    virtual void Enumerate(const std::tr1::function<void(const FileData&)> nextStage);
};

class RecursiveFileInput : public IInputProvider
{
    std::wstring root;
public:
    RecursiveFileInput(const std::wstring& rootPath);
    virtual void Enumerate(const std::tr1::function<void(const FileData&)> nextStage);
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

class SortingFilter : public IFilter
{

};

struct IOutput
{
    virtual ~IOutput() {};
    virtual void Write(const FileData&) = 0;
};

class ConsoleOutput : public IOutput
{
public:
    virtual void Write(const FileData&);
};

class ProcessKillOutput : public IOutput
{
public:
    virtual void Write(const FileData&);
};

}
