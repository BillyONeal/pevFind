//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// clsidCompressor.cpp -- Implements the clsid compressor/decompressor.
#include "pch.hpp"
#include <exception>
#include <vector>
#include <windows.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "clsidCompressor.h"
#include "utility.h"

namespace clsidCompressor {

	typedef union _clsidStructure
	{
		BYTE bytes[16];
		unsigned __int64 bigNums[2];
	} clsidStructure;

	const clsidStructure operator+(const clsidStructure& a, const clsidStructure& b);

	const clsidStructure operator-(const clsidStructure& a, const clsidStructure& b);

	int compress(const std::wstring& inFileName, const std::wstring& outFileName);
	int decompress(const std::wstring& inFileName, const std::wstring& outFileName, const bool append);
	clsidStructure createClsidFromString(const std::string& clsidString);
	std::string createStringFromClsid(const clsidStructure& clsidStruct);
	BYTE minLengthOfClsid(const clsidStructure& clsidStruct)
	{
		for(BYTE idx = 15; idx > 0; idx--)
		{
			if(clsidStruct.bytes[idx])
				return idx+1;
		}
		return 1;
	}

int main(int argc, wchar_t* argv[])
{
	//Verify 4 Arguments
	if (argc != 4)
		throw std::invalid_argument("Usage: <OPTIONS> <INFILE> <OUTFILE>");
	
	std::wstring options(argv[1]);
	std::wstring infile(argv[2]);
	std::wstring outfile(argv[3]);

	//Option: Compress
	if (options.find_first_of(L"Cc") != options.npos)
		return compress(infile, outfile);
	//Option: Decompress
	if (options.find_first_of(L"Dd") != options.npos)
	{
		if (options.find_first_of(L"Aa") == options.npos)
		{
			//Don't append to file
			return decompress(infile,outfile,false);
		} else
		{
			//Append to filee=
			return decompress(infile,outfile,true);
		}
	}
	throw std::invalid_argument("Invalid operation for CLSID Compressor.");
}

int compress(const std::wstring& inFileName, const std::wstring& outFileName)
{
	DWORD squat;
	HANDLE hFile;
	hFile = CreateFile(inFileName.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE,NULL,OPEN_EXISTING, 0,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Could not open input file.");
	DWORD bufferLength;
	bufferLength = SetFilePointer(hFile,0,0,FILE_END);
	SetFilePointer(hFile,0,0,FILE_BEGIN);
	LPBYTE inputBuffer = new BYTE[bufferLength];
	if (!ReadFile(hFile,inputBuffer,bufferLength,&squat,NULL))
	{
		throw std::runtime_error("Could not read input file.");
	}
	std::string rawFileString;
	if (IsTextUnicode(inputBuffer, bufferLength, NULL))
		rawFileString.assign(convertUnicode(std::wstring(reinterpret_cast<wchar_t *>(inputBuffer),bufferLength/sizeof(TCHAR))));
	else
		rawFileString.assign(std::string(reinterpret_cast<char *>(inputBuffer),bufferLength));
	CloseHandle(hFile);
	std::vector<std::string> clsidStrings;
	boost::algorithm::split(clsidStrings,rawFileString,boost::algorithm::is_any_of(L"\r\n"));
	for (std::vector<std::string>::iterator curLine = clsidStrings.begin(); curLine != clsidStrings.end(); curLine++)
	{
		boost::algorithm::to_upper(*curLine);
		boost::algorithm::trim(*curLine);
	}
	std::sort(clsidStrings.begin(), clsidStrings.end());
	std::vector<clsidStructure> convertedClsids;
	std::string invalidClsids;
	for (std::vector<std::string>::const_iterator it = clsidStrings.begin(); it != clsidStrings.end(); it++)
	{
		if (it->empty()) continue;
		try
		{
			//Convert the clsid into a number
			convertedClsids.push_back(createClsidFromString(*it));
		} catch (...)
		{
			//Invalid CLSID number -- add to invalid list
			invalidClsids.append(*it).append("\r\n");
		}
	}
	clsidStrings.clear();
	//Buffer for use to write later
	std::vector<BYTE> compressedBytes;

	//Length of invalid string plus length integer plus the size of the CLSID list.
	compressedBytes.reserve(invalidClsids.length() + 4 + convertedClsids.size() * 16);

	//Append length of invalid string
	union {
		DWORD dwordVal;
		BYTE bytes[4];
	} invalidLength;
	invalidLength.dwordVal = (DWORD) invalidClsids.length();
	compressedBytes.push_back((BYTE)  invalidLength.bytes[0]);
	compressedBytes.push_back((BYTE)  invalidLength.bytes[1]);
	compressedBytes.push_back((BYTE)  invalidLength.bytes[2]);
	compressedBytes.push_back((BYTE)  invalidLength.bytes[3]);
	//Append invalid string
	for(register DWORD idx = 0; idx < invalidLength.dwordVal; idx++)
	{
		compressedBytes.push_back(invalidClsids[idx]);
	}
	invalidClsids.clear();

	//Compress the 128 bit integers
	clsidStructure differenceValA;
	ZeroMemory(&differenceValA, sizeof(clsidStructure));
	clsidStructure differenceValB;
	ZeroMemory(&differenceValB, sizeof(clsidStructure));
	clsidStructure previousClsid;
	ZeroMemory(&previousClsid, sizeof(clsidStructure));
	BYTE lengthByte = 0;
	for (size_t idx = 0; idx < convertedClsids.size(); idx += 2)
	{
		//Calculate the differences
		differenceValA = convertedClsids[idx] - previousClsid;
		previousClsid = convertedClsids[idx];
		if (idx+1 < convertedClsids.size())
		{
			differenceValB = convertedClsids[idx+1] - previousClsid;
			previousClsid = convertedClsids[idx+1];
		}

		//Calculate the length byte
		lengthByte = minLengthOfClsid(differenceValA);
		if (idx+1 < convertedClsids.size())
			lengthByte |= (minLengthOfClsid(differenceValB) << 4);

		//Push the results onto our result stack.
		compressedBytes.push_back(lengthByte);
		for(BYTE clsidIdx = 0; clsidIdx < (lengthByte & 0x0F); clsidIdx++)
		{
			compressedBytes.push_back(differenceValA.bytes[clsidIdx]);
		}
		if (idx+1 < convertedClsids.size())
			for(BYTE clsidIdx = 0; clsidIdx < (lengthByte >> 4); clsidIdx++)
			{
				compressedBytes.push_back(differenceValB.bytes[clsidIdx]);
			};
	}
	convertedClsids.clear();
	if (lengthByte & 0xF0  >> 4)
		compressedBytes.push_back(0x00);

	hFile = CreateFile(outFileName.c_str(),GENERIC_WRITE, 0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Could not open output file.");
	if (!WriteFile(hFile,&compressedBytes[0],compressedBytes.size(),&squat,NULL))
		throw std::runtime_error("Could not write to output file.");
	CloseHandle(hFile);

	return 0;
}

#define WRITE_ERROR_MESSAGE std::runtime_error("Could not write to output file.")
#define READ_ERROR_MESSAGE std::runtime_error("Could not read from input file.")

int decompress(const std::wstring& inFileName, const std::wstring& outFileName, const bool append)
{
	HANDLE hInFile;
	hInFile = CreateFile(inFileName.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE,NULL,OPEN_EXISTING, 0,NULL);
	if (hInFile == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Could not open input file.");
	HANDLE hOutFile;
	hOutFile = CreateFile(outFileName.c_str(),GENERIC_WRITE, 0,NULL,OPEN_ALWAYS, 0,NULL);
	if (hOutFile == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Could not open output file.");

	if (append)
		SetFilePointer(hOutFile, 0, 0, FILE_END);
	DWORD invalidLength;
	DWORD bytesRead;
	DWORD bytesWritten;

	if (!ReadFile(hInFile,&invalidLength,4,&bytesRead,NULL))
		throw READ_ERROR_MESSAGE;
	if (bytesRead != 4)
		throw READ_ERROR_MESSAGE;

	LPBYTE invalids = new BYTE[invalidLength+1];
	if (!ReadFile(hInFile,invalids,invalidLength,&bytesRead,NULL))
		throw READ_ERROR_MESSAGE;
	if (bytesRead != invalidLength)
		throw READ_ERROR_MESSAGE;
	if (!WriteFile(hOutFile,invalids,invalidLength,&bytesWritten,NULL))
		throw WRITE_ERROR_MESSAGE;
	if (invalidLength != bytesWritten)
		throw WRITE_ERROR_MESSAGE;
	delete [] invalids;

	BYTE clsidLenByte, lengthA;
	clsidStructure clsidStruct, prevClsid;
	ZeroMemory(&prevClsid,sizeof(clsidStructure));
	for (;;)
	{
		if (!ReadFile(hInFile,&clsidLenByte,1,&bytesRead,NULL))
			throw READ_ERROR_MESSAGE;
		if (!bytesRead)
			throw READ_ERROR_MESSAGE;

		lengthA = clsidLenByte & 0x0F;
		clsidLenByte >>= 4;
		if (!lengthA)
			break;
		ZeroMemory(&clsidStruct,sizeof(clsidStruct));
		if (!ReadFile(hInFile,clsidStruct.bytes,lengthA,&bytesRead,NULL))
			throw READ_ERROR_MESSAGE;
		if (bytesRead != lengthA)
			throw READ_ERROR_MESSAGE;
		prevClsid = clsidStruct + prevClsid;
		if (!WriteFile(hOutFile,
			createStringFromClsid(prevClsid).c_str(),
			40,&bytesWritten,NULL))
			throw WRITE_ERROR_MESSAGE;
		if (bytesWritten != 40)
			throw WRITE_ERROR_MESSAGE;
		if (!clsidLenByte)
			break;
		ZeroMemory(&clsidStruct,sizeof(clsidStruct));
		if (!ReadFile(hInFile,clsidStruct.bytes,clsidLenByte,&bytesRead,NULL))
			throw READ_ERROR_MESSAGE;
		if (bytesRead != clsidLenByte)
			throw READ_ERROR_MESSAGE;
		prevClsid = clsidStruct + prevClsid;
		if (!WriteFile(hOutFile,
			createStringFromClsid(prevClsid).c_str(),
			40,&bytesWritten,NULL))
			throw WRITE_ERROR_MESSAGE;
		if (bytesWritten != 40)
			throw WRITE_ERROR_MESSAGE;
	}
	CloseHandle(hInFile);
	CloseHandle(hOutFile);
	return 0;
}
const char hexLetters[] = "0123456789ABCDEF";
clsidStructure createClsidFromString(const std::string& clsidString)
{
	//Sanity check
	if (
		(clsidString.length() != 38) ||
		clsidString[ 0] != '{' ||
		clsidString[ 9] != '-' ||
		clsidString[14] != '-' ||
		clsidString[19] != '-' ||
		clsidString[24] != '-' ||
		clsidString[37] != '}'
		)
		throw std::invalid_argument("Invalid CLSID");
	std::string workingClsidString(clsidString);

	//Remove the -s and {}s
	workingClsidString.erase(0,1);
	workingClsidString.erase(8,1);
	workingClsidString.erase(12,1);
	workingClsidString.erase(16,1);
	workingClsidString.erase(20,1);
	workingClsidString.erase(32);

	//Ensure all digits are in hex
	for(register unsigned int idx = 0; idx < 32; idx++)
	{
		if (! ((workingClsidString[idx] >= '0' && workingClsidString[idx] <= '9') || (workingClsidString[idx] >= 'A' && workingClsidString[idx] <= 'F')))
			throw std::invalid_argument("Invalid CLSID");
	}

	clsidStructure clsidStruct;

	//Convert hex to bytes
	for (register unsigned int idx = 0; idx < 16; idx++)
	{
		clsidStruct.bytes[15-idx] = (BYTE) (strchr(hexLetters,workingClsidString[idx*2]) - hexLetters);
		clsidStruct.bytes[15-idx] <<= 4;
		clsidStruct.bytes[15-idx] |= strchr(hexLetters,workingClsidString[idx*2+1]) - hexLetters;
	}

	return clsidStruct;
}

std::string createStringFromClsid(const clsidStructure& clsidStruct)
{
	std::string retVal("{",1);
	retVal.reserve(40);
	char temp[2];
	for(register unsigned int idx=0; idx < 4; idx++)
	{
		temp[1] = *(hexLetters + (clsidStruct.bytes[15-idx] & 0x0F));
		temp[0] = *(hexLetters + (clsidStruct.bytes[15-idx] >> 4));
		retVal.append(temp,2);
	}
	retVal.append("-",1);
	for(register unsigned int idx=4; idx < 6; idx++)
	{
		temp[1] = *(hexLetters + (clsidStruct.bytes[15-idx] & 0x0F));
		temp[0] = *(hexLetters + (clsidStruct.bytes[15-idx] >> 4));
		retVal.append(temp,2);
	}
	retVal.append("-",1);
	for(register unsigned int idx=6; idx < 8; idx++)
	{
		temp[1] = *(hexLetters + (clsidStruct.bytes[15-idx] & 0x0F));
		temp[0] = *(hexLetters + (clsidStruct.bytes[15-idx] >> 4));
		retVal.append(temp,2);
	}
	retVal.append("-",1);
	for(register unsigned int idx=8; idx < 10; idx++)
	{
		temp[1] = *(hexLetters + (clsidStruct.bytes[15-idx] & 0x0F));
		temp[0] = *(hexLetters + (clsidStruct.bytes[15-idx] >> 4));
		retVal.append(temp,2);
	}
	retVal.append("-",1);
	for(register unsigned int idx=10; idx < 16; idx++)
	{
		temp[1] = *(hexLetters + (clsidStruct.bytes[15-idx] & 0x0F));
		temp[0] = *(hexLetters + (clsidStruct.bytes[15-idx] >> 4));
		retVal.append(temp,2);
	}
	retVal.append("}\r\n",3);
	return retVal;
}

const clsidStructure operator-(const clsidStructure& a, const clsidStructure& b)
{
	clsidStructure ret(a);
	ret.bigNums[0] -= b.bigNums[0];
	ret.bigNums[1] -= b.bigNums[1];
    if (ret.bigNums[0] > a.bigNums[0])
        ret.bigNums[1]--;
	return ret;
}

const clsidStructure operator+(const clsidStructure& a, const clsidStructure& b)
{
	clsidStructure ret(a);
	ret.bigNums[0] += b.bigNums[0];
	ret.bigNums[1] += b.bigNums[1];
	if (ret.bigNums[0] < a.bigNums[0] || ret.bigNums[0] < b.bigNums[0])
		ret.bigNums[1]++;
	return ret;
}

} //namespace CLSIDCompressor
