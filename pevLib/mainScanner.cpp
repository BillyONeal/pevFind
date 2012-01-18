//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// mainScanner.cpp -- Implements the scanner by default.
// Recurses into subdirectories.

#include "pch.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "logger.h"
#include "utility.h"
#include "mainScanner.h"
#include "globalOptions.h"
#include "regex.h"
#include "fileData.h"
#include "zipIt.h"

namespace scanners
{

	void recursiveScanner::scan()
	{
		HANDLE hFind;
		WIN32_FIND_DATA findData;
		bool fastEcho = (!globalOptions::sortMethod[0]) && globalOptions::zipFileName.empty(); //cache whether we're able to output quickly or not

		//Create a list to hold our results
		std::list<FileData> results;

		//This list is a queue of remaining folders to scan. Initialized with the common root of the regexes
		std::list<std::wstring> foldersToScan;
		if (globalOptions::noSubDirectories)
		{
			for (std::vector<std::tr1::shared_ptr<regexClass> >::iterator it = globalOptions::regularExpressions.begin(); it != globalOptions::regularExpressions.end(); it++)
			{
				std::wstring curRegexRoot((*it)->getPathRoot());
				if ((*it)->getPathRoot().empty())
					continue;
				curRegexRoot.append(L"*");
				foldersToScan.push_back(curRegexRoot);
			}
			foldersToScan.sort();
			foldersToScan.erase(std::unique(foldersToScan.begin(), foldersToScan.end()),foldersToScan.end());
			if (foldersToScan.empty())
				foldersToScan.push_back(L"*");
		} else
		{
			std::wstring commonRoot = getRegexesCommonRoot(globalOptions::regularExpressions);
			commonRoot.append(1,L'*');
			foldersToScan.push_front(commonRoot);
		}

		do { //Go until the queue is empty
			disable64.disableFS();
			std::wstring& currentSearchDirectory(foldersToScan.front());
			// Start finding the current directory
			hFind = FindFirstFile(currentSearchDirectory.c_str(),&findData);
			// If for some reason this directory does not exist, skip it but throw no error
			if (hFind == INVALID_HANDLE_VALUE)
			{
				disable64.enableFS();
				foldersToScan.pop_front();
				continue;
			}
			//Remove the \* suffix used for the find functions
			currentSearchDirectory.erase(currentSearchDirectory.length()-1);
			if (findData.cFileName[0] == L'.' && findData.cFileName[1] == NULL)
			{
				if (!FindNextFile(hFind,&findData)) //Skip .
				{
					disable64.enableFS();
					foldersToScan.pop_front();
					continue;
				}
			}
			if (findData.cFileName[0] == L'.' && findData.cFileName[1] == L'.' && findData.cFileName[2] == NULL)
			{
				if (!FindNextFile(hFind,&findData)) //Skip ..
				{
					disable64.enableFS();
					foldersToScan.pop_front();
					continue;
				}
			}
			std::list<std::wstring>::iterator insPos = foldersToScan.begin();
			insPos++;
			do { //Loop through the current directory
				FileData currentFile(findData, currentSearchDirectory);
				//If it's a directory and it passes the tree's directory check,
				//add it to the list of directories to search
				if (!globalOptions::noSubDirectories)
				{
					if (currentFile.isDirectory())
					{	
						if (globalOptions::logicalTree->directoryCheck(currentFile.getFileName()))
						{
							std::wstring newDir(currentFile.getFileName());
							newDir.append(L"\\*");
							foldersToScan.insert(insPos,newDir);
						}
					}
				}
				//If the tree says this file doesn't match, go ahead and check the next one
				if (!globalOptions::logicalTree->include(currentFile))
					continue;
				//If we're sorting, store the file into the results list for sorting later.
				//Otherwise just print it now
				if (fastEcho)
					currentFile.write();
				else
					results.push_back(currentFile);
			} while (FindNextFile(hFind,&findData)); //While there's anything left
			FindClose(hFind);
			disable64.enableFS();
			foldersToScan.pop_front();
		} while(!foldersToScan.empty()); //Go until the queue is empty
		//If we're sorting, sort and print the results
		if (globalOptions::sortMethod[0])
		{
			results.sort();
			for(std::list<FileData>::iterator it = results.begin(); it != results.end(); it++)
			{
				it->write();
			}
		}
		if (!globalOptions::zipFileName.empty()) //If there's a zip file name, do the zip.
			zipIt(globalOptions::zipFileName, results);
		printSummary();
	}

	std::wstring getRegexesCommonRoot(std::vector<std::tr1::shared_ptr<regexClass> >& targets)
	{
		//Sanity check:
		if (!targets.size()) return L"";
		std::vector<std::wstring> candidates;
		candidates.reserve(targets.size());
		//Keep the roots of the regexes in the running if they are not empty.
		for(std::vector<std::tr1::shared_ptr<regexClass> >::const_iterator it = targets.begin(); it != targets.end(); it++)
		{
			if (! (**it).getPathRoot().empty() ) candidates.push_back((**it).getPathRoot());
		}
		switch(candidates.size())
		{
			//No regexes with a root. Therefore we are starting in the current
			//working directory. Return an empty string.
		case 0:
			return L"";
			//Only one regex with a root. Therefore that root must be the one we
			//are looking for.
		case 1:
			return candidates[0];
		}
		std::size_t endingIndex = 0;
		for(; endingIndex != candidates[0].length(); endingIndex++)
		{
			wchar_t currentCharacter = towlower(candidates[0][endingIndex]);
			for(std::vector<std::wstring>::const_iterator it = candidates.begin(); it != candidates.end(); it++)
			{
				if (it->size() <= endingIndex) goto done;
				if (towlower(it->at(endingIndex)) != currentCharacter) goto done;
			}
		}
		done:
		std::wstring::const_iterator endOfCommonality;
		endOfCommonality = std::find(candidates[0].rend() - endingIndex, candidates[0].rend(), L'\\').base();
		return std::wstring(candidates[0].begin(), endOfCommonality);
	}

	void printSummary()
	{
		if (!globalOptions::summary)
			return;
		logger << L"\r\n Entries:     " << rightPad(getSizeString(globalOptions::totalEntries),12)
			<< L"  (" << getSizeString(globalOptions::visibleEntries)
			<< L")\r\n Directories: " << rightPad(getSizeString(globalOptions::visibleDirs),12)
			<< L"  Files:  " << rightPad(getSizeString(globalOptions::visibleFiles),12)
			<< L"\r\n Bytes:       " << rightPad(getSizeString(globalOptions::totalSize),12)
			<< L"  Blocks: " << rightPad(getSizeString(globalOptions::blocks),12) << L"\r\n";
	}
}; // Namespace scanners