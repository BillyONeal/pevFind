#include "pch.hpp"
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlwapi.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "filter.h"
#include "fileData.h"
#include "utility.h"

std::wstring sizeFilter::debugTreeInternal(const std::wstring& type) const
{
	return std::wstring(L"+ SIZEFILTER ").append(type).append(L" ").append(boost::lexical_cast<std::wstring>(size));
}
unsigned __int32 sizeFilter::getPriorityClass() const
{
	return PRIORITY_SLOW_FILTER;
}
sizeFilter::sizeFilter(unsigned __int64 createdSize) : size(createdSize)
{}
gtSizeFilter::gtSizeFilter(unsigned __int64 createdSize) : sizeFilter(createdSize)
{}
BOOL gtSizeFilter::include(FileData &file) const
{
	return file.getSize() > size;
}
std::wstring gtSizeFilter::debugTree() const
{
	return debugTreeInternal(L"GREATERTHAN");
}
ltSizeFilter::ltSizeFilter(unsigned __int64 createdSize) : sizeFilter(createdSize)
{}
BOOL ltSizeFilter::include(FileData &file) const
{
	return file.getSize() < size;
}
std::wstring ltSizeFilter::debugTree() const
{
	return debugTreeInternal(L"LESSTHAN");
}
notSizeFilter::notSizeFilter(unsigned __int64 createdSize) : sizeFilter(createdSize)
{}
BOOL notSizeFilter::include(FileData &file) const
{
	return file.getSize() != size;
}
std::wstring notSizeFilter::debugTree() const
{
	return debugTreeInternal(L"ISNOT");
}
isSizeFilter::isSizeFilter(unsigned __int64 createdSize) : sizeFilter(createdSize)
{}
BOOL isSizeFilter::include(FileData &file) const
{
	return file.getSize() == size;
}
std::wstring isSizeFilter::debugTree() const
{
	return debugTreeInternal(L"EQUALS");
}
unsigned __int32 dateFilter::getPriorityClass() const
{
	return PRIORITY_SLOW_FILTER;
}
dateFilter::dateFilter(const FILETIME &inDate) : date(inDate)
{}
accessLDate::accessLDate(const FILETIME &inDate) : dateFilter(inDate)
{}
std::wstring accessLDate::debugTree() const
{
	return L"ACCESS DATEFILTER LESSTHAN " + getDateAsString(date);
}
BOOL accessLDate::include(FileData &file) const
{
	return file.getLastAccessTime() < date;
}
accessGDate::accessGDate(const FILETIME &inDate) : dateFilter(inDate)
{}
std::wstring accessGDate::debugTree() const
{
	return L"ACCESS DATEFILTER GREATERTHAN " + getDateAsString(date);
}
BOOL accessGDate::include(FileData &file) const
{
	return date < file.getLastAccessTime();
}
modifiedLDate::modifiedLDate(const FILETIME &inDate) : dateFilter(inDate)
{}
std::wstring modifiedLDate::debugTree() const
{
	return L"MOD DATEFILTER LESSTHAN " + getDateAsString(date);
}
BOOL modifiedLDate::include(FileData &file) const
{
	return file.getLastModTime() < date;
}
modifiedGDate::modifiedGDate(const FILETIME &inDate) : dateFilter(inDate)
{}
std::wstring modifiedGDate::debugTree() const
{
	return L"MOD DATEFILTER GREATERTHAN " + getDateAsString(date);
}
BOOL modifiedGDate::include(FileData &file) const
{
	return date < file.getLastModTime();
}
createdLDate::createdLDate(const FILETIME &inDate) : dateFilter(inDate)
{}
std::wstring createdLDate::debugTree() const
{
	return L"CREATED DATEFILTER LESSTHAN " + getDateAsString(date);
}
BOOL createdLDate::include(FileData &file) const
{
	return file.getCreationTime() < date;
}
createdGDate::createdGDate(const FILETIME &inDate) : dateFilter(inDate)
{}
std::wstring createdGDate::debugTree() const
{
	return L"CREATED DATEFILTER GREATERTHAN " + getDateAsString(date);
}
BOOL createdGDate::include(FileData &file) const
{
	return date < file.getCreationTime();
}
unsigned __int32 fastFilter::getPriorityClass() const
{
	return PRIORITY_FAST_FILTER;
}
BOOL isArchive::include(FileData &file) const
{
	return file.isArchive();
}
std::wstring isArchive::debugTree() const
{
	return std::wstring(L"+ ISARCHIVE");
}
BOOL isCompressed::include(FileData &file) const
{
	return file.isCompressed();
}
std::wstring isCompressed::debugTree() const
{
	return std::wstring(L"+ ISCOMPRESSED");
}
BOOL isDirectory::include(FileData &file) const
{
	return file.isDirectory();
}
std::wstring isDirectory::debugTree() const
{
	return std::wstring(L"+ ISDIRECTORY");
}
BOOL isFile::include(FileData &file) const
{
	return file.isFile();
}
std::wstring isFile::debugTree() const
{
	return std::wstring(L"+ ISFILE");
}
BOOL isReparsePoint::include(FileData &file) const
{
	return file.isReparsePoint();
}
std::wstring isReparsePoint::debugTree() const
{
	return std::wstring(L"+ ISREPARSEPOINT");
}
BOOL isSFCProtected::include(FileData &file) const
{
	return file.isSfcProtected();
}
std::wstring isSFCProtected::debugTree() const
{
	return std::wstring(L"+ ISSFCPROTECTED");
}
BOOL isHidden::include(FileData &file) const
{
	return file.isHidden();
}
std::wstring isHidden::debugTree() const
{
	return std::wstring(L"+ ISHIDDEN");
}
BOOL isReadOnly::include(FileData &file) const
{
	return file.isReadOnly();
}
std::wstring isReadOnly::debugTree() const
{
	return std::wstring(L"+ ISREADONLY");
}
BOOL isSystem::include(FileData &file) const
{
	return file.isSystem();
}
std::wstring isSystem::debugTree() const
{
	return std::wstring(L"+ ISSYSTEM"); 
}
BOOL isVolumeLabel::include(FileData &file) const
{
	return file.isVolumeLabel();
}
std::wstring isVolumeLabel::debugTree() const
{
	return std::wstring(L"+ ISVOLLABEL");
}
BOOL isWritable::include(FileData &file) const
{
	return file.isWritable();
}
std::wstring isWritable::debugTree() const
{
	return std::wstring(L"+ ISWRITABLE");
}
BOOL isTemp::include(FileData &file) const
{
	return file.isTemporary();
}
std::wstring isTemp::debugTree() const
{
	return std::wstring(L"+ ISTEMP");
}
unsigned __int32 sigIsValid::getPriorityClass() const
{
	return PRIORITY_SIGCHECK;
}
BOOL sigIsValid::include(FileData &file) const
{
	return file.hasValidDigitalSignature();
}
std::wstring sigIsValid::debugTree() const
{
	return std::wstring(L"+ SIGVALID");
}
unsigned __int32 peFilter::getPriorityClass() const
{
	return PRIORITY_PE_DATA;
}
BOOL hasSig::include(FileData &file) const
{
	return file.hasAuthenticodeSignature();
}
std::wstring hasSig::debugTree() const
{
	return std::wstring(L"+ HASSIGNATURE");
}
BOOL isPEFile::include(FileData &file) const
{
	return file.isPE();
}
std::wstring isPEFile::debugTree() const
{
	return std::wstring(L"+ ISPORTABLEEXE");
}
BOOL timestampValid::include(FileData &file) const
{
	return file.peHeaderTimeIsValid();
}
std::wstring timestampValid::debugTree() const
{
	return std::wstring(L"+ PETIMESTAMPVALID");
}
BOOL checkSumValid::include(FileData &file) const
{
	return file.peHeaderChecksumIsValid(); 
}
std::wstring checkSumValid::debugTree() const
{
	return std::wstring(L"+ CHECKSUMVALID");
}
BOOL isDLLFile::include(FileData &file) const
{
	return file.isDLL();
}
std::wstring isDLLFile::debugTree() const
{
	return std::wstring(L"+ ISDLL");
}
unsigned __int32 hash::getPriorityClass() const
{
	return PRIORITY_HASH_CHECK;
}
BOOL md5Match::include(FileData &file) const
{
	return file.MD5() == md5Val;
}
std::wstring md5Match::debugTree() const
{
	return std::wstring(L"+ MD5 MATCHES ").append(md5Val);
}
md5Match::md5Match(std::wstring md5Value): md5Val(md5Value)
{
	boost::algorithm::to_upper(md5Val);
}
BOOL sha1Match::include(FileData &file) const
{
	return file.SHA1() == sha1Val;
}
std::wstring sha1Match::debugTree() const
{
	return std::wstring(L"+ SHA-1 MATCHES ").append(sha1Val);
}
sha1Match::sha1Match(std::wstring sha1Value): sha1Val(sha1Value)
{
	boost::algorithm::to_upper(sha1Val);
}
std::wstring hashList::listValues() const
{
	std::wstring retVal;
	for( std::vector<std::wstring>::const_iterator it = values.begin(); it != values.end(); it++)
		retVal.append(L"  ").append(*it).append(L"\r\n");
	retVal.erase(retVal.end() - 2, retVal.end());
	return retVal;
}
hashList::hashList(std::vector<std::wstring> hashes) : values(hashes)
{
	std::sort(values.begin(),values.end());
	for(std::vector<std::wstring>::iterator it = hashes.begin(); it != hashes.end(); it++)
	{
		boost::algorithm::to_upper(*it);
	}
}
BOOL md5List::include(FileData &file) const
{
	return std::binary_search(values.begin(),values.end(),file.MD5());
}
std::wstring md5List::debugTree() const
{
	return L"+ MD5 LIST:\r\n" + listValues();
}
md5List::md5List(std::vector<std::wstring> md5s): hashList(md5s)
{}
BOOL sha1List::include(FileData &file) const
{
	return std::binary_search(values.begin(),values.end(),file.SHA1());
}
std::wstring sha1List::debugTree() const
{
	return L"+ SHA-1 LIST:\r\n" + listValues();
}
sha1List::sha1List(std::vector<std::wstring> sha1s): hashList(sha1s)
{}
BOOL md5EList::include(FileData &file) const
{
	std::wstring calcHash = file.MD5();
	if (calcHash[0] == L'!')
		return true;
	return std::binary_search(values.begin(),values.end(),calcHash);
}
std::wstring md5EList::debugTree() const
{
	return L"+ MD5 OR ERROR LIST:\r\n" + listValues();
}
md5EList::md5EList(std::vector<std::wstring> md5s): hashList(md5s)
{}
BOOL sha1EList::include(FileData &file) const
{
	std::wstring calcHash = file.SHA1();
	if (calcHash[0] == L'!')
		return true;
	return std::binary_search(values.begin(),values.end(),calcHash);
}
std::wstring sha1EList::debugTree() const
{
	return L"+ SHA-1 OR ERROR LIST:\r\n" + listValues();
}
sha1EList::sha1EList(std::vector<std::wstring> sha1s): hashList(sha1s)
{}
unsigned __int32 skipper::getPriorityClass() const 
{ 
	return PRIORITY_FAST_FILTER; 
}
BOOL skipper::include(FileData & /*file*/) const
{ 
	return true; 
}
std::wstring skipper::debugTree() const 
{ 
	return L"SKIP " + _toSkip;
}
unsigned int skipper::directoryCheck(const std::wstring& directory)
{
	wchar_t relPath[MAX_PATH];
	if (!PathRelativePathTo(relPath, _toSkip.c_str(), FILE_ATTRIBUTE_DIRECTORY, directory.c_str(), FILE_ATTRIBUTE_DIRECTORY))
		return DIRECTORY_DONTCARE;
	if (relPath[0] == L'.' && relPath[1] == NULL)
		return DIRECTORY_EXCLUDE;
	return DIRECTORY_DONTCARE;
}
skipper::skipper(const std::wstring& toSkip) : _toSkip(toSkip)
{}

BOOL isNEFile::include(FileData &file) const
{
	return file.isNE();
}

std::wstring isNEFile::debugTree() const
{
	return std::wstring(L"+ ISNEWEXE");
}

BOOL isLEFile::include(FileData &file) const
{
	return file.isLE();
}

std::wstring isLEFile::debugTree() const
{
	return std::wstring(L"+ ISLEFORMATEXE");
}

BOOL isMZFile::include(FileData &file) const
{
	return file.isMZ();
}

std::wstring isMZFile::debugTree() const
{
	return std::wstring(L"+ ISMZCOMPLIANTEXE");
}

BOOL is2ExecFile::include(FileData &file) const
{
	return file.isStrongExecutable();
}

std::wstring is2ExecFile::debugTree() const
{
	return std::wstring(L"+ ISSTRONGTYPEEXE");
}

unsigned __int32 headerLDate::getPriorityClass() const
{
	return PRIORITY_PE_DATA;
}

std::wstring headerLDate::debugTree() const
{
	return L"HEADER DATEFILTER LESSTHAN " + getDateAsString(date);
}

BOOL headerLDate::include(FileData &file) const
{
	if (!file.isPE())
		return false;
	return file.getPEHeaderTime() < date;
}

unsigned __int32 headerGDate::getPriorityClass() const
{
	return PRIORITY_PE_DATA;
}

std::wstring headerGDate::debugTree() const
{
	return L"HEADER DATEFILTER GREATERTHAN " + getDateAsString(date);
}

BOOL headerGDate::include(FileData &file) const
{
	if (!file.isPE())
		return false;
	return date < file.getPEHeaderTime();
}

BOOL isPEPlusFile::include( FileData &file ) const
{
	return file.isPEPlus();
}

std::wstring isPEPlusFile::debugTree() const
{
	return std::wstring(L"+ IS64BITPORTABLEEXE");
}
