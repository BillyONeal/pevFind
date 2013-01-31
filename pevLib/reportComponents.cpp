//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "pch.hpp"
#include "fileData.h"
#include "reportComponents.hpp"

namespace
{
    class ScopedFindHandle
    {
        HANDLE hInner;
    public:
        ScopedFindHandle(HANDLE hArg) : hInner(hArg)
        { }
        HANDLE Get()
        {
            return hInner;
        }
        ~ScopedFindHandle()
        {
            if (hInner == INVALID_HANDLE_VALUE)
            {
                return;
            }
            BOOL val = FindClose(hInner);
            assert(val);
        }
    };

    struct VectorFileDataAppender : public std::unary_function<void, const FileData&>
    {
        std::vector<FileData>& inserted;
    public:
        void operator()(const FileData& toInsert)
        {
            inserted.push_back(toInsert);
        }
        VectorFileDataAppender(std::vector<FileData>& toInsertTo) : inserted(toInsertTo)
        { }
    };
}

namespace vFind {

std::vector<FileData> IFilter::Results()
{
    return std::vector<FileData>();
}

FileInput::FileInput(const std::wstring& rootPath) : root(rootPath)
{
}

void FileInput::Enumerate(const std::tr1::function<void(const FileData&)> nextStage)
{
    std::wstring rootPlusWildcard;
    rootPlusWildcard.reserve(root.size() + 2);
    rootPlusWildcard.append(root).append(L"\\*");
    WIN32_FIND_DATAW data;
    ScopedFindHandle hSearch(FindFirstFileW(rootPlusWildcard.c_str(), &data));
    if (hSearch.Get() == INVALID_HANDLE_VALUE)
    {
        return;
    }
    do
    {
        //Throw out . and ..
        if (data.cFileName[0] == L'.' && (data.cFileName[1] == L'\0' || (data.cFileName[1] == L'.' && data.cFileName[2] == L'\0')))
        {
            continue;
        }
        FileData answer(data, root);
        nextStage(answer);
    } while (FindNextFileW(hSearch.Get(), &data));
}

RecursiveFileInput::RecursiveFileInput(const std::wstring& rootPath) : root(rootPath)
{ }

static void RecursiveSearch(const std::wstring& root, const std::tr1::function<void(const FileData&)> nextStage)
{
    std::vector<FileData> currentSearchResults;
    FileInput thisSearch(root);
    thisSearch.Enumerate(VectorFileDataAppender(currentSearchResults));
    std::vector<FileData>::iterator it = currentSearchResults.begin();
    for (; it != currentSearchResults.end(); ++it)
    {
        nextStage(*it);
        if (it->isDirectory())
        {
            RecursiveSearch(it->getFileName(), nextStage);
        }
    }
}

void RecursiveFileInput::Enumerate(const std::tr1::function<void(const FileData&)> nextStage)
{
    RecursiveSearch(root, nextStage);
}

}