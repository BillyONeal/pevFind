//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "pch.hpp"
#include "ExpressionParser.hpp"
#include "../LogCommon/Win32Exception.hpp"
#include "utility.h"
#include <cassert>

static std::wstring emptyString;

namespace pevFind
{
    LoadedFile::LoadedFile(SourceLocation startLocation_, std::wstring input_, std::wstring name_) throw()
        : startLocation(startLocation_)
        , input(std::move(input_))
        , name(std::move(name_))
    { }
    LoadedFile::LoadedFile(LoadedFile&& other) throw()
        : startLocation(other.startLocation)
        , input(std::move(other.input))
        , name(std::move(other.name))
    {
    }
    LoadedFile& LoadedFile::operator=(LoadedFile&& other) throw()
    {
        startLocation = other.startLocation;
        input = std::move(other.input);
        name = std::move(other.name);
        return *this;
    }
    SourceLocation LoadedFile::GetStartLocation() const throw() { return startLocation; }
    std::uint32_t LoadedFile::size() const throw() { return static_cast<std::uint32_t>(input.size()); }
    wchar_t LoadedFile::operator[](std::size_t idx) const throw() { return input[idx]; }
    wchar_t const* LoadedFile::data() const throw() { return input.data(); }

    LoadedFile const& SourceManager::GetBufferForLocation(SourceLocation) const throw()
    {
        return loadedFiles[0];
    }

    DecomposedSourceLocation SourceManager::GetDecomposedLocation(SourceLocation loc) const throw()
    {
        DecomposedSourceLocation result = { &loadedFiles[0], loc };
        return std::move(result);
    }

    void SourceManager::InstallFile(LoadedFile&& file)
    {
        loadedFiles.emplace_back(std::move(file));
    }

    std::wstring SourceManager::GetSpellingOfRange(SourceLocation first, SourceLocation last) const
    {
        assert(last > first);
        auto const len = last - first;
        std::wstring result(len, L'\0');
        DecomposedSourceLocation resultBuffer = GetDecomposedLocation(first);
        std::copy_n(resultBuffer.file->data() + resultBuffer.relativeLocation, len, result.begin());
        return result;
    }

    LoadLineResult::LoadLineResult(std::wstring&& lineOrError_, bool success_)
        : lineOrError(std::move(lineOrError_))
        , success(success_)
    {
    }

    LoadLineResult LoadLineResult::FromLineValue(std::wstring line)
    {
        return LoadLineResult(std::move(line), true);
    }

    LoadLineResult LoadLineResult::FromFailure(std::wstring error)
    {
        return LoadLineResult(std::move(error), false);
    }

    std::wstring const& LoadLineResult::GetLine() const throw()
    {
        if (success)
        {
            return lineOrError;
        }
        else
        {
            return emptyString;
        }
    }

    std::wstring&& LoadLineResult::StealLine() throw()
    {
        assert(Success());
        return std::move(lineOrError);
    }

    bool LoadLineResult::Success() const
    {
        return success;
    }

    std::wstring const& LoadLineResult::GetError() const throw()
    {
        if (success)
        {
            return emptyString;
        }
        else
        {
            return lineOrError;
        }
    }

    ILoadLineResolver::~ILoadLineResolver() throw()
    {
    }

    LoadLineResult FileLoadLineResolver::LoadLineByName(std::wstring const& name) const
    {
        try
        {
            return LoadLineResult::FromLineValue(loadFileAsString(name));
        }
        catch (std::bad_alloc const&)
        {
            return LoadLineResult::FromFailure(L"The file \"" + name + L"\" was too large to load as a command line source.");
        }
        catch (Instalog::SystemFacades::Win32Exception const& error)
        {
            return LoadLineResult::FromFailure(L"Failed opening \"" + name + L"\" with error " + error.GetWideMessage());
        }
    }

    void PreconfiguredLoadLineResolver::Add(wchar_t const* name, wchar_t const* value)
    {
        lines.insert(std::make_pair(name, value));
    }

    LoadLineResult PreconfiguredLoadLineResolver::LoadLineByName(std::wstring const& name) const
    {
        auto const namePosition = lines.find(name);
        if (namePosition == lines.cend())
        {
            return LoadLineResult::FromFailure(L"Line was not configured.");
        }
        else
        {
            return LoadLineResult::FromLineValue(namePosition->second);
        }
    }
}
