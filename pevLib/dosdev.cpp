//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// dosdev.cpp -- Implements the "ddev" sub program.

#include "pch.hpp"
#include <string>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <vector>
#include <windows.h>
#include "dosdev.h"
#include "logger.h"

namespace dosdev {

bool StringIsDriveLetter(const std::wstring& in);

struct deviceSorter_ {
	bool operator()(const std::wstring& a, const std::wstring& b)
	{
		bool aIsDriveLetter = StringIsDriveLetter(a);
		bool bIsDriveLetter = StringIsDriveLetter(b);
		if (aIsDriveLetter && !bIsDriveLetter) return true;
		if (!aIsDriveLetter && bIsDriveLetter) return false;
		return a < b;
	}
} deviceSorter;

bool StringIsDriveLetter(const std::wstring& in)
{
	if (in.length() != 2)  return false;
	if (! iswalpha(in[0])) return false;
	if (in[1] != L':')     return false;
	return true;
}

void displayDevice(const std::wstring &in);

int main(int argc, wchar_t* argv[])
{
	bool all = false;
	bool systemDriveSource = false;
	DWORD flags = 0;
	std::wstring sourcePath;
	std::wstring targetPath;
	argc--; argv++; //Remove program name
	
	while(argc)
	{
		//Accept if it's a switch -- that is, it begins with - or /
		if (wcslen(argv[0]) > 1 && (argv[0][0] == L'-' || argv[0][0] == L'/'))
		{
			//Loop through specified switches.
			for (wchar_t * strIt = argv[0] + 1; strIt != argv[0] + wcslen(argv[0]); strIt++)
			{
				switch (*strIt)
				{
				case L'A':
				case L'a':
					all = true;
					break;
				case L'S':
				case L's':
					systemDriveSource = true;
					break;
				case L'R':
				case L'r':
					flags |= DDD_RAW_TARGET_PATH;
					break;
				case L'D':
				case L'd':
					flags |= DDD_REMOVE_DEFINITION;
					break;
				case L'E':
				case L'e':
					flags |= DDD_EXACT_MATCH_ON_REMOVE;
					break;
				case L'N':
				case L'n':
					flags |= DDD_NO_BROADCAST_SYSTEM;
				default:
					throw std::runtime_error("Unrecognised option to DOSDEV!");
				}
			}
		} else //NOT a switch
		{
			sourcePath = argv[0];
			if (argc > 1)
			{
				targetPath = argv[1];
			}
			break; //Commandline processing finished, break out of the FOR
		}
		argc--; argv++;
	}
	if (systemDriveSource)
	{
		wchar_t systemDrive[4];
		targetPath = sourcePath;
		if (!ExpandEnvironmentStrings(L"%SYSTEMDRIVE%", systemDrive, 3))
			throw std::runtime_error("Systemdrive could not be enumerated.");
		sourcePath.assign(systemDrive);
	}
	if (sourcePath.length()) //We are defining a device
	{
		DefineDosDevice(flags, sourcePath.c_str(), targetPath.empty() ? NULL : targetPath.c_str());
	}
	//Print device list
	std::vector<std::wstring> driveStrings;
	wchar_t *drivesBuffer = NULL;
	DWORD error = 0;
	DWORD currentSize = 0;
	const DWORD incrementSize = 2048;
	do {
		delete [] drivesBuffer;
		currentSize += incrementSize;
		drivesBuffer = new wchar_t[currentSize];
		error = QueryDosDevice(NULL, drivesBuffer, currentSize);
	} while (error == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return -1;
	for(wchar_t *bufferWalker = drivesBuffer; *bufferWalker; bufferWalker += wcslen(bufferWalker) + 1)
	{
		driveStrings.push_back(bufferWalker);
	}
	delete [] drivesBuffer;
	if (driveStrings.empty()) return 0;
	std::sort(driveStrings.begin(), driveStrings.end(), deviceSorter);
	std::vector<std::wstring>::const_iterator stringsWalker = driveStrings.begin();
	for (; stringsWalker != driveStrings.end() && StringIsDriveLetter(*stringsWalker); stringsWalker++)
	{
		displayDevice(*stringsWalker);
	}
	if (all && stringsWalker != driveStrings.end())
	{
		logger << L"\r\n";
		for (; stringsWalker != driveStrings.end(); stringsWalker++)
		{
			displayDevice(*stringsWalker);
		}
	}
#ifndef NDEBUG
	system("pause");
#endif
	return 0;
}

void displayDevice(const std::wstring &in)
{
	wchar_t * targetBuffer = NULL;
	DWORD currentSize = 0;
	DWORD error = 0;
	const DWORD incrementSize = 2048;
	//Display device path
	logger << in;
	do {
		delete [] targetBuffer;
		currentSize += incrementSize;
		targetBuffer = new wchar_t[currentSize];
		error = QueryDosDevice(in.c_str(), targetBuffer, currentSize);
	} while (error == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		logger << L"\r\n";
		delete [] targetBuffer;
		return;
	}
	logger << L" = " << targetBuffer;
	delete [] targetBuffer;
	UINT result = GetDriveType(in.c_str());
	switch (result)
	{
	case DRIVE_REMOVABLE:
		logger << L" [Removable]";
		break;
	case DRIVE_FIXED:
		logger << L" [Fixed]";
		break;
	case DRIVE_REMOTE:
		logger << L" [Remote]";
		break;
	case DRIVE_CDROM:
		logger << L" [CDRom]";
		break;
	case DRIVE_RAMDISK:
		logger << L" [RAMDisk]";
		break;
	}
	logger << L"\r\n";
}
} //namespace dosdev
