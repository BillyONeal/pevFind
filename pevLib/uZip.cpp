//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// uZip.cpp -- Implements the uZip subprogram.

#include "pch.hpp"
#include <string>
#include "unzip.h"
#include "logger.h"
#include "uzip.h"

namespace uZip {

int main(int argc, wchar_t* argv[])
{
	static const char invalidCmd[] = "Invalid commandline.";
	static const char fileTarget[] = "Target is a file!";
	static const char cantOpen[] = "Cannot open ZIP file!";
	static const char cantWrite[] = "There was an error writing the unzipped file.";

	if (argc < 2 || argc > 4)
		throw std::runtime_error(invalidCmd);

	bool createFolders = false;
	bool list = false;

	if (wcslen(argv[1]) >= 2  && (argv[1][0] == L'-' || argv[1][0] == L'/') && (argv[1][0] == L'D' || argv[1][0] == L'd'))
	{
		createFolders = true;
	}

	std::wstring inFile;
	std::wstring outDir;

	if (createFolders)
	{
		if (argc == 3)
			list = true;
	} else
	{
		if (argc == 2)
			list = true;
	}

	if (!list)
	{
		if (createFolders)
		{
			inFile.assign(argv[2]);
			outDir.assign(argv[3]);
		}
		else
		{
			inFile.assign(argv[1]);
			outDir.assign(argv[2]);
		}
	} else
	{
		inFile.assign(argv[1]);
	}

	HZIP hZip = OpenZip(inFile.c_str(), NULL);
	if (!hZip)
		throw std::runtime_error(cantOpen);

	ZIPENTRY zipAttributes;
	GetZipItem(hZip, -1, &zipAttributes);
	for (int idx = 0; idx < zipAttributes.index; idx++)
	{
		ZIPENTRY currentItem;
		GetZipItem(hZip, idx, &currentItem);
		if (currentItem.attr & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		if (list)
		{
			logger << currentItem.name;
			logger << L"\r\n";
		} else
		{
			std::wstring currentItemPath(currentItem.name);
			if (!createFolders)
			{
				size_t fileStart = currentItemPath.rfind(L'/');
				if (fileStart != currentItemPath.npos)
					currentItemPath.erase(0, fileStart + 1);
			}
			currentItemPath.insert(0, L"\\");
			currentItemPath.insert(0, outDir);
			ZRESULT zRes;
			zRes = UnzipItem(hZip, idx, currentItemPath.c_str());
			if (zRes && zRes != ZR_FLATE)
				throw std::runtime_error(cantWrite);
		}
	}
	CloseZip(hZip);
	return 0;
}

} //namespace Uzip
