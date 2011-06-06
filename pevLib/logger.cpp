//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// logger.cpp -- Implements character conversions and file handling.
#include <string>
#include <windows.h>
#include "logger.h"
#include "globalOptions.h"

HANDLE logger_class::stdOut;
bool logger_class::useWriteConsole;

logger_class& logger_class::operator<<(const std::wstring& rhs)
{
	DWORD squat;
	BOOL blank;
	int len;
	char *toWrite;
	if (useWriteConsole)
	{
		WriteConsole(stdOut,rhs.c_str(),rhs.length(),&squat,NULL);
		return *this;
	}
	switch(globalOptions::encoding)
	{
	case globalOptions::ENCODING_TYPE_ACP:
		len = WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,rhs.c_str(),rhs.length(),NULL,NULL,"?",&blank);
		toWrite = new char[len];
		WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,rhs.c_str(),rhs.length(),toWrite,len,"?",&blank);
		WriteFile(stdOut,toWrite,len,&squat,NULL);
		delete [] toWrite;
		break;
	case globalOptions::ENCODING_TYPE_OEM:
		len = WideCharToMultiByte(CP_OEMCP,WC_NO_BEST_FIT_CHARS,rhs.c_str(),rhs.length(),NULL,NULL,"?",&blank);
		toWrite = new char[len];
		WideCharToMultiByte(CP_OEMCP,WC_NO_BEST_FIT_CHARS,rhs.c_str(),rhs.length(),toWrite,len,"?",&blank);
		WriteFile(stdOut,toWrite,len,&squat,NULL);
		delete [] toWrite;
		break;
	case globalOptions::ENCODING_TYPE_UTF8:
		len = WideCharToMultiByte(CP_UTF8,NULL,rhs.c_str(),rhs.length(),NULL,NULL,NULL,NULL);
		toWrite = new char[len];
		WideCharToMultiByte(CP_UTF8,NULL,rhs.c_str(),rhs.length(),toWrite,len,NULL,NULL);
		WriteFile(stdOut,toWrite,len,&squat,NULL);
		delete [] toWrite;
		break;
	case globalOptions::ENCODING_TYPE_UTF16:
	default:
		WriteFile(stdOut,rhs.c_str(),rhs.length()*sizeof(TCHAR),&squat,NULL);
		break;
	}
	return *this;
}
void logger_class::update(const std::wstring& fileName)
{
	HANDLE newFile;
	useWriteConsole = false;
	newFile = CreateFile(fileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (newFile == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("Could not set output file.");
	} else
	{
		CloseHandle(stdOut);
		stdOut = newFile;
	}
}
logger_class::logger_class()
{
	DWORD squat;
	stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	useWriteConsole = false;
	if (GetConsoleMode(stdOut,&squat))
		useWriteConsole = true;
}
logger_class::~logger_class()
{
	if (stdOut != INVALID_HANDLE_VALUE)
	CloseHandle(stdOut);
	stdOut = INVALID_HANDLE_VALUE;
}