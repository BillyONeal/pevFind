//          Copyright Billy O'Neal 2011
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// fileData.cpp -- Implements functions for the fileData record type.

#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <windows.h>
#include <shlwapi.h>
#include <Wincrypt.h>
#include <Mscat.h>
#include <softpub.h>
#include <Sfc.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include "utility.h"
#include "logger.h"
#include "fileData.h"
#include "globalOptions.h"

//Constants
const wchar_t constantHexArray[] = L"0123456789ABCDEF";

//Constructors
// Build filedata records
FileData::FileData(const WIN32_FIND_DATA &rawData, const std::basic_string<TCHAR>& root)
{
	bits = 0;

	//Copy the contents of the win32finddata structure to our internals
	fileName = rawData.cFileName;
	setAttributesAccordingToDWORD(rawData.dwFileAttributes);

	//Add the path root to the current filename
	fileName.insert(0,root);
}
FileData::FileData(const std::wstring& fileNameBuild) : fileName(fileNameBuild), versionInformationBlock(NULL)
{
	bits = 0;
}

//Sort function. Sorts are tried until either a mismatch is found, or the end of user specified sorts
//is found.
//
// This function is complex because it is possible to sort based on more than one metric
bool FileData::operator<(FileData& rhs)
{
	globalOptions::sorts *sortPointer = globalOptions::sortMethod;
	if (! (*sortPointer))
		return false;
	do
	{
		switch(*sortPointer)
		{
			// Ascending sorts
		case globalOptions::SIZE: //Size
			if (getSize() != rhs.getSize())
				return getSize() > rhs.getSize();
			break;
		case globalOptions::NAME: //Name
			if (fileName != rhs.getFileName())
			{
				return ! boost::algorithm::lexicographical_compare(fileName,rhs.getFileName());
			}
			break;
		case globalOptions::INAME: //Case insensitive name
			if (fileName != rhs.getFileName())
			{
				return ! boost::algorithm::ilexicographical_compare(fileName,rhs.getFileName());
			}
			break;
		case globalOptions::ADATE: //Access date
			if (getLastAccessTime() != rhs.getLastAccessTime())
				return getLastAccessTime() > rhs.getLastAccessTime();
			break;
		case globalOptions::MDATE: //Modified date
			if (getLastModTime() != rhs.getLastModTime())
				return getLastModTime() > rhs.getLastModTime();
			break;
		case globalOptions::CDATE: //Created date
			if (getCreationTime() != rhs.getCreationTime())
				return getCreationTime() > rhs.getCreationTime();
			break;
		case globalOptions::HDATE: //PE Header date
			if (getPEHeaderTime() != rhs.getPEHeaderTime())
				return getPEHeaderTime() < rhs.getPEHeaderTime();
			break;
			//Descending sorts
		case globalOptions::DSIZE: //Size
			if (getSize() != rhs.getSize())
				return getSize() < rhs.getSize();
			break;
		case globalOptions::DNAME: //Name
			if (fileName != rhs.getFileName())
			{
				return boost::algorithm::lexicographical_compare(fileName,rhs.getFileName());
			}
			break;
		case globalOptions::DINAME: //Case insensitive name
			if (fileName != rhs.getFileName())
			{
				return boost::algorithm::ilexicographical_compare(fileName,rhs.getFileName());
			}
			break;
		case globalOptions::DADATE: //Access date
			if (getLastAccessTime() != rhs.getLastAccessTime())
				return getLastAccessTime() < rhs.getLastAccessTime();
			break;
		case globalOptions::DMDATE: //Modified date
			if (getLastModTime() != rhs.getLastModTime())
				return getLastModTime() < rhs.getLastModTime();
			break;
		case globalOptions::DCDATE: //Created date
			if (getCreationTime() != rhs.getCreationTime())
				return getCreationTime() < rhs.getCreationTime();
			break;
		case globalOptions::DHDATE: //PE Header date
			if (getPEHeaderTime() != rhs.getPEHeaderTime())
				return getPEHeaderTime() > rhs.getPEHeaderTime();
			break;
		default:
			return 0;
		}
	} while (*sortPointer++);
	return 0;
}
std::basic_string<TCHAR> FileData::getAttributesString() const
{
	std::basic_string<TCHAR> result;
	result.reserve(7);
	if (bits & DIRECTORY)
		result.append(1, L'd');
	else if (bits & VOLLABEL)
		result.append(1, L'v');
	else
		result.append(1, L'-');
	appendAttributeCharacter(result, L'c', COMPRESSED);
	appendAttributeCharacter(result, L's', SYSTEM);
	appendAttributeCharacter(result, L'h', HIDDEN);
	appendAttributeCharacter(result, L'a', ARCHIVE);
	appendAttributeCharacter(result, L't', TEMPORARY);
	if (bits & READONLY)
		result.append(1, L'r');
	else
		result.append(1, L'w');
	appendAttributeCharacter(result, L'e', REPARSE);
	return result;
}
std::basic_string<TCHAR> FileData::getPEAttribsString() const
{
	if (!(bits & PEENUMERATED))
		initPortableExecutable();
	std::basic_string<TCHAR> result;
	result.reserve(6);
	appendAttributeCharacter(result, L'1', ISPE);
	appendAttributeCharacter(result, L'2', DEBUG);
	appendAttributeCharacter(result, L'3', SIGPRESENT);
	appendAttributeCharacter(result, L'4', DLL);
	if (peHeaderChecksumIsValid())
		result.append(1, L'5');
	else
		result.append(1, L'-');
	if (peHeaderTimeIsValid())
		result.append(1, L'6');
	else
		result.append(1, L'-');
	return result;
}

void inline FileData::appendAttributeCharacter(std::basic_string<TCHAR> &result, const TCHAR attributeCharacter, const size_t curBit) const
{
	if (bits & curBit)
		result.append(1, attributeCharacter);
	else
		result.append(1, L'-');
}
void FileData::setAttributesAccordingToDWORD(DWORD win32Attribs) const
{
	if (win32Attribs & FILE_ATTRIBUTE_ARCHIVE)
		bits |= ARCHIVE;
	if (win32Attribs & FILE_ATTRIBUTE_COMPRESSED)
		bits |= COMPRESSED;
	if (win32Attribs & FILE_ATTRIBUTE_DIRECTORY)
		bits |= DIRECTORY;
	else
		bits |= FILE;
	if (win32Attribs & FILE_ATTRIBUTE_HIDDEN)
		bits |= HIDDEN;
	if (win32Attribs & FILE_ATTRIBUTE_READONLY)
		bits |= READONLY;
	else
		bits |= WRITABLE;
	if (win32Attribs & FILE_ATTRIBUTE_SYSTEM)
		bits |= SYSTEM;
	if (win32Attribs & FILE_ATTRIBUTE_TEMPORARY)
		bits |= TEMPORARY;
	if (win32Attribs & FILE_ATTRIBUTE_REPARSE_POINT)
		bits |= REPARSE;
	bits |= WIN32ENUMD;
}
void FileData::initPortableExecutable() const
{
	if (bits & PEENUMERATED)
		return;
	//Store that this function has been called so that it is not called every time
	bits |= PEENUMERATED;
	//Get a handle to the file
	boost::shared_ptr<void> hFile(getFileHandle(), CloseHandle);
	//Report false if the file could not be obtained
	if (hFile.get() == INVALID_HANDLE_VALUE)
		return;

	//A common DWORD for remembering the length of returned file data.
	DWORD lengthRead = 0;

	//Check for the MZ signature at the beginning of the PE file.
	BYTE mzCheck[2];
	ReadFile(hFile.get(), mzCheck, 2, &lengthRead, NULL);
	if (lengthRead != 2 || memcmp(mzCheck, "MZ", 2))
		return;

	bits |= ISMZ;

	//Seek to offset 0x3C,which contains the offset to the PE header
	if (SetFilePointer(hFile.get(), 0x3c, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return;
	//Get the offset
	LONG peOffset;
	if (!ReadFile(hFile.get(), &peOffset, sizeof(LONG), &lengthRead, NULL))
		return;
	//Seek to the offset
	if (SetFilePointer(hFile.get(), peOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return;

	//Check for PE Signature
	BYTE peSig[4];
	ReadFile(hFile.get(), peSig, 4, &lengthRead, NULL);
	if (lengthRead != 4)
		return;

	if (!memcmp(peSig, "NE", 2))
	{
		bits |= ISNE;
	} else if (!memcmp(peSig, "LE", 2))
	{
		bits |= ISLE;
	} else if (!memcmp(peSig, "PE\0", 4))
	{
		//Get the IMAGE_FILE_HEADER
		IMAGE_FILE_HEADER fileHeader;
		memset(&fileHeader, 0, sizeof(IMAGE_FILE_HEADER));
		if (!ReadFile(hFile.get(), &fileHeader, sizeof(IMAGE_FILE_HEADER), &lengthRead, NULL))
			return;

		//Extract PE Header timestamp
		headerTime = UnixTimeToFileTime(fileHeader.TimeDateStamp);

		//Set characteristics bits
		if (!(fileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED))
			bits |= DEBUG;
		if (fileHeader.Characteristics & IMAGE_FILE_DLL)
			bits |= DLL;

		//Read the magic number from the optional header
		BYTE optionalHeaderMagic[2];
		if (!ReadFile(hFile.get(), optionalHeaderMagic, 2, &lengthRead, NULL))
			return;

		//Check for valid magic
		if (optionalHeaderMagic[0] != 0x0B)
			return;

		//Hold which PE version it is -- PE32 or PE32+.
		//PE32+ is used on x64 machines.
		bool isPEPlus;

		//Calculate the above using the magic value
		if      (optionalHeaderMagic[1] == 0x01)
			isPEPlus = false;
		else if (optionalHeaderMagic[1] == 0x02)
			isPEPlus = true;
		else
			return;

		//All three magic numbers are correct here, we have a PE file on our hands
		bits |= ISPE;

		//Get the base address of the image for future calculations. 64 bit value for PE32+
		__int64 imageBaseAddress = 0;

		if (isPEPlus)
		{
			//In PE32+, the offset for the base address is 24, and is eight bytes long
			if (SetFilePointer(hFile.get(), 22, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
				return;
			if (!ReadFile(hFile.get(), &imageBaseAddress, 8, &lengthRead, NULL))
				return;
			bits |= PEPLUS;
		} else
		{
			//In PE32, the offset for the base address is 28, and is four bytes long.
			if (SetFilePointer(hFile.get(), 26, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
				return;
			if (!ReadFile(hFile.get(), &imageBaseAddress, 4, &lengthRead, NULL))
				return;
		}

		//Find headerSum
		//Offset from the base address is 32 (64 from begining of the optional header)
		if (SetFilePointer(hFile.get(), 32, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			return;
		//Read headerSum
		if (!ReadFile(hFile.get(), &headerSum, sizeof(DWORD), &lengthRead, NULL))
			return;

		//Find number of PE data sections
		DWORD numberOfSections;
		if (isPEPlus)
		{
			if (SetFilePointer(hFile.get(), 32, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
				return;
		} else
		{
			if (SetFilePointer(hFile.get(), 40, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
				return;
		}
		//Read NumberOfRvaAndSizes
		if (!ReadFile(hFile.get(), &numberOfSections, sizeof(DWORD), &lengthRead, NULL))
			return;

		//There can be no signature in the file if the number of sections is less than 5,
		//because the certificate table is the 5th section.
		if (numberOfSections < 5)
			return;

		//Check for certificates
		//Look for "Certificate Table", 32 bytes from the RvaAndSizes
		if (SetFilePointer(hFile.get(), 32, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			return;

		//The certificate table pointer is 8 bytes long -- it will be all zeros if the table is not present.
		DWORD CertVirtualAddress, CertSize;
		if (!ReadFile(hFile.get(), &CertVirtualAddress, sizeof(DWORD), &lengthRead, NULL))
			return;
		if (!ReadFile(hFile.get(), &CertSize, sizeof(DWORD), &lengthRead, NULL))
			return;

		//If the size of the certificate section is not 0, set the sigpresent flag.
		if (CertSize)
			bits |= SIGPRESENT;
	}

}

void FileData::sigVerify() const
{
	HANDLE hFile = getFileHandle();
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	WINTRUST_FILE_INFO FileData;
	memset(&FileData, 0, sizeof(WINTRUST_FILE_INFO));
	FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
	FileData.hFile = hFile;
	WINTRUST_DATA trustData;
	memset(&trustData, 0, sizeof(WINTRUST_DATA));
	trustData.cbStruct = sizeof(WINTRUST_DATA);
	trustData.dwUIChoice = WTD_UI_NONE;
	trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
	trustData.dwUnionChoice = WTD_CHOICE_FILE;
	trustData.dwProvFlags = WTD_SAFER_FLAG;
	trustData.pFile = &FileData;
	//End of MSDN defaults
	GUID policyGuid = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	LONG result = WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE),&policyGuid,&trustData);
	switch(result)
	{
	case ERROR_SUCCESS:				//Verified sucessfully
		bits |= SIGPRESENT;
		bits |= SIGVALID;
		break;
	case CRYPT_E_SECURITY_SETTINGS: //Error with Crypto, but sig present
		bits |= CRYPTSVCERROR;
		break;
	case TRUST_E_EXPLICIT_DISTRUST: //Sig invalid
	case TRUST_E_SUBJECT_NOT_TRUSTED:
		bits |= SIGPRESENT;
		bits &= (~SIGVALID);
		break;
	default:
		{
			//Something else wrong -- likely windows file without authenticode.
			trustData.dwUnionChoice = WTD_CHOICE_CATALOG;
			WINTRUST_CATALOG_INFO catInfo;
			trustData.pCatalog = &catInfo;
			catInfo.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
			catInfo.pcwszMemberFilePath = fileName.c_str();
			catInfo.hMemberFile = hFile;
			HCATADMIN catalogHandler;
			// Get a context to the system catalog database
			CryptCATAdminAcquireContext(&catalogHandler,NULL,NULL);
			DWORD hashLen = 0;
			//Calculate the file's hash. Get length of required hash len.
			CryptCATAdminCalcHashFromFileHandle(hFile,&hashLen,NULL,NULL);
			//Alocate ram for and calculate hash.
			boost::scoped_array<BYTE> hash(new BYTE[hashLen]);
			CryptCATAdminCalcHashFromFileHandle(hFile,&hashLen,hash.get(),NULL);
			//Get the catalog to which that hash corresponds.
			HCATINFO hCatInfo = CryptCATAdminEnumCatalogFromHash(catalogHandler,hash.get(),hashLen,NULL,NULL);
			if (!hCatInfo)
			{
				//The hash is not in any catalog
				bits &= (~SIGVALID);
				CloseHandle(hFile);
				CryptCATAdminReleaseContext(catalogHandler,NULL);
				return;
			} else
			{
				//Clean up our refrence to the context and catalog
				CryptCATAdminReleaseCatalogContext(catalogHandler,hCatInfo,NULL);
				CryptCATAdminReleaseContext(catalogHandler,NULL);
				bits |= SIGVALID;
				CloseHandle(hFile);
				return;
			}
		}
	}
	CloseHandle(hFile);
}
#pragma warning (push)
#pragma warning (disable: 4706)
std::wstring FileData::MD5() const
{
	return getHash<CryptoPP::Weak::MD5>();
}
std::wstring FileData::SHA1() const
{
	return getHash<CryptoPP::SHA1>();
}
std::wstring FileData::SHA224() const
{
	return getHash<CryptoPP::SHA224>();
}
std::wstring FileData::SHA256() const
{
	return getHash<CryptoPP::SHA256>();
}
std::wstring FileData::SHA384() const
{
	return getHash<CryptoPP::SHA384>();
}
std::wstring FileData::SHA512() const
{
	return getHash<CryptoPP::SHA512>();
}
#pragma warning (pop)
void FileData::enumVersionInformationBlock() const
{
	bits |= VERSIONINFOCHECKED;
	wchar_t filePathBuffer[MAX_PATH];
	if (!PathSearchAndQualify(getFileName().c_str(), filePathBuffer, MAX_PATH)) return;
	disable64.disableFS();
	DWORD zero = 0;
	DWORD lengthOfVersionData =
	GetFileVersionInfoSize(filePathBuffer,&zero);
	if (!lengthOfVersionData)
	{
		disable64.enableFS();
		return;
	}
	versionInformationBlock.resize(lengthOfVersionData);
	GetFileVersionInfo(filePathBuffer,zero,lengthOfVersionData,&versionInformationBlock[0]);
	LANGANDCODEPAGE *languageBlock;
	UINT translationsCount;
	VerQueryValue(&versionInformationBlock[0],L"\\VarFileInfo\\Translation",(LPVOID*)&languageBlock,&translationsCount);
	translationsCount /= sizeof(struct LANGANDCODEPAGE);
	versionTranslations.assign(languageBlock, languageBlock + translationsCount);
	LANGANDCODEPAGE enUs;
	enUs.wCodePage = 1200;
	enUs.wLanguage = 1033;
	versionTranslations.push_back(enUs);
	enUs.wCodePage = 0x04E4;
	versionTranslations.push_back(enUs);
	std::sort(versionTranslations.begin(), versionTranslations.end());
	versionTranslations.erase(
		std::unique(versionTranslations.begin(), versionTranslations.end()),
		versionTranslations.end());
	disable64.enableFS();
}
std::wstring FileData::getVersionInformationString(const std::wstring& requestedResourceType) const
{
	if (!(bits & VERSIONINFOCHECKED))
		enumVersionInformationBlock();
	if (versionInformationBlock.empty())
		return L"------";
	LPCTSTR retString;
	UINT retStringLength;
	std::wstring result;
	for( size_t idx = 0; idx < versionTranslations.size(); idx++ )
	{
		std::wstringstream versionQuery;
		versionQuery << L"\\StringFileInfo\\";
		versionQuery << std::hex << std::setw(4) << std::setfill(L'0')
			<< versionTranslations[idx].wLanguage;
		versionQuery << std::hex << std::setw(4) << std::setfill(L'0')
			<< versionTranslations[idx].wCodePage;
		versionQuery << L'\\' << requestedResourceType;
		std::wstring versionQueryString(versionQuery.str());
		if (!VerQueryValue(&versionInformationBlock[0],versionQueryString.c_str(),(LPVOID *)&retString,&retStringLength))
		{
			DWORD error = GetLastError();
			if (error == 1813)
				continue;
			std::wstringstream errorMessage;
			errorMessage << L"ERROR: 0x" << std::hex << error;
			return errorMessage.str();
		}
		if (!result.empty())
			result.append(L" / ");
		std::wstring tempString(retString);
		boost::algorithm::trim(tempString);
		result.append(tempString);
	}
	return result;
}


HANDLE FileData::getFileHandle(bool readOnly) const
{
	disable64.disableFS();
	HANDLE result = CreateFile(
		fileName.c_str(),
		readOnly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL
	);
	disable64.enableFS();
	return result;
}
/************************************************************************************
 *
 * Disclaimer -- the checksum function is not my code. I used example code
 * from this codeproject article -> http://www.codeproject.com/KB/cpp/PEChecksum.aspx
 * This is because pevFind cannot require IMAGEHLP.DLL
 *
 ************************************************************************************/
#pragma warning (push)
#pragma warning (disable: 4244)
WORD FileData::ChkSum(WORD oldChk, USHORT * ptr, DWORD len) const
{
	DWORD c = oldChk;
	while (len)
	{
		int l = min(len, 0x4000);
		len -= l;
		for (int j=0; j<l; ++j)
		{
			c += *ptr++;
		}
		c = (c&0xffff) + (c>>16);
	}
	c = (c&0xffff) + (c>>16);
	return (WORD)c;
}

DWORD FileData::GetPEChkSum(LPCTSTR filename) const
{
	HANDLE hf = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (hf == INVALID_HANDLE_VALUE) return 0;

	DWORD dwRead = 0;
	DWORD dwSize = GetFileSize(hf, 0);
	IMAGE_DOS_HEADER dosh;
	IMAGE_NT_HEADERS nth;
	ReadFile(hf, &dosh, sizeof(dosh), &dwRead, 0);
	SetFilePointer(hf, dosh.e_lfanew, 0, FILE_BEGIN);
	ReadFile(hf, &nth, sizeof(nth), &dwRead, 0);
	SetFilePointer(hf, 0, 0, FILE_BEGIN);

	const int sz = 0x10000;
	void * mem = malloc(sz);
	DWORD dwCheck = 0;

	dwRead = 0;

	while (ReadFile(hf, mem, sz, &dwRead, 0) && dwRead > 0)
	{
		dwCheck = ChkSum(dwCheck, (PUSHORT)mem, dwRead/2);
	}

	if (dwRead & 1)
	{
		dwCheck += ((BYTE*)mem)[dwRead-1];
		dwCheck = (dwCheck>>16) + (dwCheck&0xffff);
	}
	free(mem);
	CloseHandle(hf);

	DWORD yy = 0;
	if (dwCheck-1 < nth.OptionalHeader.CheckSum)
	{
		yy = (dwCheck-1) - nth.OptionalHeader.CheckSum;
	}
	else
	{
		yy = dwCheck - nth.OptionalHeader.CheckSum;
	}
	yy = (yy&0xffff) + (yy>>16);
	yy = (yy&0xffff) + (yy>>16);
	yy += dwSize;
	return yy;
}
#pragma warning(pop)

std::vector<std::wstring> FileData::sfcFileStrings;
unsigned int FileData::sfcState = NOT_CHECKED;

typedef struct _PPROTECT_FILE_ENTRY {
  PWSTR SourceFileName;
  PWSTR FileName;
  PWSTR InfName;
}PROTECT_FILE_ENTRY, *PPROTECT_FILE_ENTRY;

typedef NTSTATUS (WINAPI *_SfcGetFiles)(
  PPROTECT_FILE_ENTRY *ProtFileData,
  PULONG FileCount
);

void FileData::buildSfcList() const
{
	//Load the SFCFiles.dll module.
	wchar_t buffer[MAX_PATH];
	ExpandEnvironmentStrings(L"%WINDIR%\\System32\\sfcfiles.dll", buffer, MAX_PATH);
	boost::shared_ptr<void> sfcFiles(LoadLibrary(buffer), FreeLibrary);
	if (!(sfcFiles.get()))
	{
		sfcState = NO_SFCFILES_DLL;
		return;
	}
	_SfcGetFiles SfcGetFiles;
	SfcGetFiles = (_SfcGetFiles)GetProcAddress((HMODULE)sfcFiles.get(), "SfcGetFiles");
	if (!SfcGetFiles)
	{
		sfcState = NO_SFCFILES_DLL;
		return;
	}
	NTSTATUS getFilesResult;
	PROTECT_FILE_ENTRY *fileEntry;
	ULONG fileCount = 0;
	getFilesResult = SfcGetFiles(&fileEntry, &fileCount);
	if (getFilesResult != 0)
	{	
		sfcState = NO_SFCFILES_DLL;
		return;
	}
	sfcFileStrings.reserve(fileCount);
	for(ULONG idx = 0; idx < fileCount; idx++)
	{
		ExpandEnvironmentStrings(fileEntry->FileName, buffer, MAX_PATH);
		std::wstring convertedBuffer(buffer);
		boost::algorithm::to_upper(convertedBuffer);
		sfcFileStrings.push_back(convertedBuffer);
		fileEntry++;
	}
	std::sort(sfcFileStrings.begin(), sfcFileStrings.end());
	sfcState = ENUMERATED;
	return;
}

bool FileData::isSfcProtected() const
{
	// SFCState maintains whether we have checked for SFC files or not yet. If not, build
	// the list of SFC files from the running system. If that cannot be done,
	// (as will be the case on vista which has no SFCFiles.DLL) fail over to Windows'
	// SfcIsFileProtected function, which is Vista's implementation that works
	// in Safe Mode.
	switch (sfcState)
	{
	case NOT_CHECKED:
		buildSfcList();
		return isSfcProtected();
	case NO_SFCFILES_DLL:
		return SfcIsFileProtected(NULL,getFileName().c_str()) != 0;
	case ENUMERATED:
		{
			std::wstring upCase;
			upCase = boost::algorithm::to_upper_copy(getFileName());
			return std::binary_search(sfcFileStrings.begin(), sfcFileStrings.end(), upCase);
		}
	default:
		return false;
	}
}

void FileData::resetPEHeaderCheckSum()
{
	if (isPE() && peHeaderChecksumIsValid() && getPEHeaderCheckSum() != 0) return;

	//Get a handle to the file
	boost::shared_ptr<void> hFile(getFileHandle(false), CloseHandle);
	//Report false if the file could not be obtained
	if (hFile.get() == INVALID_HANDLE_VALUE)
		return;

	//A common DWORD for remembering the length of returned file data.
	DWORD lengthRead = 0;

	//Check for the MZ signature at the beginning of the PE file.
	BYTE mzCheck[2];
	ReadFile(hFile.get(), mzCheck, 2, &lengthRead, NULL);
	if (lengthRead != 2 || memcmp(mzCheck, "MZ", 2))
		return;

	//Seek to offset 0x3C,which contains the offset to the PE header
	if (SetFilePointer(hFile.get(), 0x3c, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return;
	//Get the offset
	LONG peOffset;
	if (!ReadFile(hFile.get(), &peOffset, sizeof(LONG), &lengthRead, NULL))
		return;
	//Seek to the offset
	if (SetFilePointer(hFile.get(), peOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return;

	//Check for PE Signature
	BYTE peSig[4];
	ReadFile(hFile.get(), peSig, 4, &lengthRead, NULL);
	if (lengthRead != 4)
		return;

	if (memcmp(peSig, "PE\0", 4))
		return;
	//Get the IMAGE_FILE_HEADER
	IMAGE_FILE_HEADER fileHeader;
	memset(&fileHeader, 0, sizeof(IMAGE_FILE_HEADER));
	if (!ReadFile(hFile.get(), &fileHeader, sizeof(IMAGE_FILE_HEADER), &lengthRead, NULL))
		return;

	//Read the magic number from the optional header
	BYTE optionalHeaderMagic[2];
	if (!ReadFile(hFile.get(), optionalHeaderMagic, 2, &lengthRead, NULL))
		return;

	//Check for valid magic
	if (optionalHeaderMagic[0] != 0x0B)
		return;

	//Hold which PE version it is -- PE32 or PE32+.
	//PE32+ is used on x64 machines.
	bool isPEPlus;

	//Calculate the above using the magic value
	if      (optionalHeaderMagic[1] == 0x01)
		isPEPlus = false;
	else if (optionalHeaderMagic[1] == 0x02)
		isPEPlus = true;
	else
		return;

	//Get the base address of the image for future calculations. 64 bit value for PE32+
	__int64 imageBaseAddress = 0;

	if (isPEPlus)
	{
		//In PE32+, the offset for the base address is 24, and is eight bytes long
		if (SetFilePointer(hFile.get(), 22, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			return;
		if (!ReadFile(hFile.get(), &imageBaseAddress, 8, &lengthRead, NULL))
			return;
	} else
	{
		//In PE32, the offset for the base address is 28, and is four bytes long.
		if (SetFilePointer(hFile.get(), 26, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			return;
		if (!ReadFile(hFile.get(), &imageBaseAddress, 4, &lengthRead, NULL))
			return;
	}

	//Find headerSum
	//Offset from the base address is 32 (64 from begining of the optional header)
	if (SetFilePointer(hFile.get(), 32, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
		return;
	
	DWORD realSum = getPECalculatedCheckSum();
		
	//Write new headerSum
	if (!WriteFile(hFile.get(), &realSum, sizeof(DWORD), &lengthRead, NULL))
		return;
		
	//Well the sum is correct now ;)
	headerSum = realSum;
}

void FileData::write()
{
	//Terminate if we've reached our line limit
	if (!globalOptions::lineLimit)
		return;
	globalOptions::lineLimit--;
	globalOptions::totalEntries++;
	if (!isHidden())
		globalOptions::visibleEntries++;
	if (isDirectory())
		globalOptions::visibleDirs++;
	else
		globalOptions::visibleFiles++;
	globalOptions::totalSize += getSize();
	globalOptions::blocks += (getSize() + 511) / 512;
	std::wstring line;
	TCHAR const *cursor = globalOptions::displaySpecification.c_str();
	static TCHAR checksumTemp[11];
	while (*cursor)
	{
		if (*cursor == L'#')
		{
			cursor++;
			switch (*cursor)
			{
			case L'1':
				line.append(SHA1());
				break;
			case L'2':
				line.append(SHA224());
				break;
			case L'3':
				line.append(SHA256());
				break;
			case L'4':
				line.append(SHA384());
				break;
			case L'5':
				line.append(MD5());
				break;
			case L'6':
				line.append(SHA512());
				break;
			case L'7':
				line.push_back(isPEPlus() ? L'7' : L'-');
				break;
			case L'8':
				{
					std::wstring shortPath;
					getShortPathName(getFileName(),shortPath);
					line.append(shortPath);
				}
				break;
			case L'a':
			case L'A':
				line.append(getDateAsString(getLastAccessTime()));
				break;
			case L'b':
			case L'B':
				line.push_back(L'\t');
				break;
			case L'c':
			case L'C':
				line.append(getDateAsString(getCreationTime()));
				break;
			case L'D':
			case L'd':
				line.append(GetVerCompany());
				break;
			case L'E':
			case L'e':
				line.append(GetVerDescription());
				break;
			case L'f':
			case L'F':
				line.append(getFileName());
				break;
			case L'G':
			case L'g':
				line.append(GetVerVersion());
				break;
			case L'H':
			case L'h':
				line.append(getDateAsString(getPEHeaderTime()));
				break;
			case L'I':
			case L'i':
				line.append(GetVerProductName());
				break;
			case L'J':
			case L'j':
				line.append(GetVerCopyright());
				break;
			case L'K':
			case L'k':
				line.append(GetVerOriginalFileName());
				break;
			case L'L':
			case L'l':
				line.append(GetVerTrademark());
				break;
			case L'm':
			case L'M':
				line.append(getDateAsString(getLastModTime()));
				break;
			case L'n':
			case L'N':
				line.append(L"\r\n");
				break;
			case L'O':
			case L'o':
				line.append(GetVerInternalName());
				break;
			case L'P':
			case L'p':
				line.append(getPEAttribsString());
				break;
			case L'Q':
			case L'q':
				line.append(GetVerComments());
				break;
			case L'R':
			case L'r':
				line.append(GetVerPrivateBuild());
				break;
			case L's':
			case L'S':
				line.append(rightPad(getSizeString(getSize()),17));
				break;
			case L't':
			case L'T':
				line.append(getAttributesString());
				break;
			case L'u':
			case L'U':
				if(isDirectory())
					line.append(L"--------");
				else
					line.append(boost::lexical_cast<std::wstring >(getSize()));
				break;
			case L'V':
			case L'v':
				line.push_back(hasValidDigitalSignature() ? '7' : '-');
				break;
			case L'W':
			case L'w':
				line.push_back(isSfcProtected() ? '8' : '-');
				break;
			case L'X':
			case L'x':
				line.append(GetVerSpecialBuild());
				break;
			case L'Y':
			case L'y':
				wsprintf(checksumTemp,L"0x%0.8X",getPEHeaderCheckSum());
				line.append(checksumTemp);
				break;
			case L'Z':
			case L'z':
				wsprintf(checksumTemp,L"0x%0.8X",getPECalculatedCheckSum());
				line.append(checksumTemp);
				break;
			case L'#':
			default:
				line.append(cursor,cursor+1);
			};
		} else
		{
			line.append(cursor,cursor+1);
		}
		cursor++;
	}
	logger << line << L"\r\n";
}
