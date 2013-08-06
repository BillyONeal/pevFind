//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "pch.hpp"
#include "ExpressionParser.hpp"
#include "../LogCommon/Win32Exception.hpp"
#include "utility.h"
#include <stdexcept>
#include <cassert>

static std::wstring emptyString;

namespace pevFind
{
    LoadedFile::LoadedFile(SourceLocation startLocation_, SourceLocation length_, std::wstring name_) throw()
        : startLocation(startLocation_)
        , length(length_)
        , name(std::move(name_))
    { }
    LoadedFile::LoadedFile(LoadedFile&& other) throw()
        : startLocation(other.startLocation)
        , length(other.length)
        , name(std::move(other.name))
    {
    }
    LoadedFile& LoadedFile::operator=(LoadedFile&& other) throw()
    {
        startLocation = other.startLocation;
        length = other.length;
        name = std::move(other.name);
        return *this;
    }
    SourceLocation LoadedFile::GetStartLocation() const throw() { return startLocation; }
    SourceLocation LoadedFile::GetLength() const throw() { return length; }
    void LoadedFile::AddLength(std::int32_t addedLength) throw()
    {
        length += addedLength;
    }

    SourceManager::SourceManager(std::wstring startContent, std::wstring startName)
        : backingBuffer(std::move(startContent))
    {
        if (startContent.size() > std::numeric_limits<SourceLocation>::max())
        {
            throw std::out_of_range("Start content exceeded maximum possible source location.");
        }

        loadedFiles.emplace_back(0, static_cast<SourceLocation>(backingBuffer.size()), std::move(startName));
    }

    void SourceManager::InstallFile(SourceLocation insertionLocation, SourceLocation replaceLength, std::wstring const& content, std::wstring name)
    {
        if (content.size() > std::numeric_limits<SourceLocation>::max())
        {
            throw std::out_of_range("Content exceeded maximum possible source location.");
        }

        auto contentSize = static_cast<SourceLocation>(content.size());
        if (contentSize == 0)
        {
            return;
        }

        auto newSize = contentSize + static_cast<SourceLocation>(backingBuffer.size());
        if (newSize < backingBuffer.size() || newSize < contentSize) // overflow occured
        {
            throw std::out_of_range("Backing buffer exceeded maximum command line length.");
        }

        backingBuffer.replace(insertionLocation, replaceLength, content);

        // Update lengths of the existing file records.
        for (auto& file : loadedFiles)
        {
            auto const fileBegin = file.GetStartLocation();
            auto const fileEnd = file.GetLogicalEnd();
            if (fileBegin <= insertionLocation && insertionLocation < fileEnd)
            {
                file.AddLength(contentSize - replaceLength);
            }
        }

        // Install the new file record.
        // Yes, this is insertion sort, which is normally bad. However, in the average case
        // files are inserted in sorted order already, so this is linear time for that case.
        if (insertionLocation > loadedFiles[loadedFiles.size() - 1].GetStartLocation())
        {
            loadedFiles.emplace_back(insertionLocation, contentSize, std::move(name));
        }
        else
        {
            auto const fileLocation = std::lower_bound(loadedFiles.cbegin(), loadedFiles.cend(), insertionLocation,
                [](LoadedFile const& file, SourceLocation insertLoc) { return file.GetStartLocation() < insertLoc; });
            loadedFiles.emplace(fileLocation, insertionLocation, contentSize, std::move(name));
        }
    }

    wchar_t SourceManager::operator[](SourceLocation location) const throw()
    {
        return backingBuffer[location];
    }

    std::wstring SourceManager::GenerateSourceListing(SourceLocation startLocation, SourceLocation endLocation) const
    {
        assert(startLocation <= endLocation);
        std::wstring result;
        std::vector<std::wstring> tooLongFiles;
        std::uint32_t nextFile = 1;
        std::wstring currentLine;
        currentLine.reserve((endLocation - startLocation) + 1);

        // File records are sorted by start location.
        for (LoadedFile const& file : loadedFiles)
        {
            SourceLocation begin = file.GetStartLocation();
            SourceLocation end = file.GetLogicalEnd();

            // Check if the file isn't in the output at all.
            if (endLocation < begin || end < startLocation)
            {
                continue;
            }

            // Clamp the file range to the displayed region.
            bool beginClamped = begin < startLocation;
            if (beginClamped)
            {
                begin = startLocation;
            }

            bool endClamped = endLocation < end;
            if (endClamped)
            {
                end = endLocation;
            }

            // Shift the range such that the beginning of the displayed region is zero.
            begin -= startLocation;
            end -= startLocation;
            
            // Make sure the file is in the display region.
            if (begin >= end)
            {
                continue;
            }

            // Move to the next line if necessary.
            if (currentLine.size() > begin)
            {
                currentLine.push_back(L'\n');
                result.append(currentLine);
                currentLine.clear();
            }

            // Get the number to print.
            bool tooLong = true;
            auto numeral = std::to_wstring(nextFile++);
            auto availableSpace = end-begin;
            if (numeral.size() > availableSpace)
            {
                // Not enough space to print the number. Give up.
                numeral.resize(1);
                numeral[0] = L'?';
            }

            auto const numeralSize = numeral.size();
            // Fill the preceeding area with whitespace.
            currentLine.resize(begin, L' ');

            // Choose how to print the source reference.
            if (availableSpace == numeralSize)
            {
                // Just enough space for the number
                currentLine.append(numeral);
            }
            else if (availableSpace == numeralSize + 1)
            {
                // Just enough space for the number and ^
                currentLine.append(numeral);
                currentLine.push_back(L'^');
            }
            else if (availableSpace == numeralSize + 2)
            {
                // Just enough space for the number and end indicators
                currentLine.push_back(beginClamped ? L'<' : L'|');
                currentLine.append(numeral);
                currentLine.push_back(endClamped ? L'>' : L'|');
            }
            else /* if (availableSpace == numeralSize + 3) */
            {
                // Enough space for the general algorithm
                currentLine.push_back(beginClamped ? L'<' : L'|');
                currentLine.append(numeral);
                currentLine.push_back(L'~');
                std::wstring const& name = file.GetName();
                auto const maxNameLength = availableSpace - numeralSize - 3;
                if (name.size() > maxNameLength)
                {
                    // Not enough space for the full name
                    currentLine.append(name.cend() - maxNameLength, name.cend());
                }
                else
                {
                    // Just enough or more for the full name
                    tooLong = false;
                    currentLine.append(name);
                    currentLine.resize(end - 1, L'-');
                }

                currentLine.push_back(endClamped ? L'>' : L'|');
            }

            // File was too long, remember it for later.
            if (tooLong)
            {
                std::wstring tooLongEntry;
                tooLongEntry.reserve(numeralSize + 2 + file.GetName().size() + 1);
                tooLongEntry.push_back(L'\n');
                tooLongEntry.append(numeral).append(L": ").append(file.GetName());
                tooLongFiles.emplace_back(std::move(tooLongEntry));
            }
        }

        if (!currentLine.empty())
        {
            result.append(currentLine);
        }

        for (std::wstring const& line : tooLongFiles)
        {
            result.append(line);
        }

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

    static bool IsWhitespace(wchar_t ch)
    {
        return ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r';
    }

    SourceLocation GetTokenStartAfter(SourceManager const& sm, SourceLocation startAt)
    {
        for(;;)
        {
            if (!sm.CharacterIsAt(startAt))
            {
                break;
            }

            if (!IsWhitespace(sm[startAt]))
            {
                break;
            }

            ++startAt;
        }

        return startAt;
    }
}
