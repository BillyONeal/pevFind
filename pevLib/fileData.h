//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// fileData.h -- The header for one file record. This class
// contains methods for signature verification, timestamp
// comparison, PE file detection, among other things.
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <strsafe.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp562/md5.h>
#include <cryptopp562/sha.h>
#include "utility.h"

class FileData
{
    //Defines for the individual bits in the bitset containing properties for this filedata object
    enum
    {
        ARCHIVE =                0x00000001,  // Standard Win32 Attributes
        COMPRESSED =             0x00000002,
        DIRECTORY =             0x00000004,
        FILE =                    0x00000008,
        HIDDEN =                0x00000010,
        READONLY =                 0x00000020,
        SYSTEM =                 0x00000040,
        VOLLABEL =                 0x00000080,
        WRITABLE =                 0x00000100,
        REPARSE =                 0x00000200,
        TEMPORARY =             0x00000400,
        WIN32ENUMD =             0x00000800,
        // END Standard Win32 Attributes
        // Signature Attributes
        SIGVALID =                 0x00002000,
        // Executable Attributes
        DLL =                     0x00004000,
        DEBUG =                 0x00008000,
        ISPE =                     0x00010000,
        ISNE =                     0x00020000,
        ISLE =                     0x00040000,
        ISMZ =                     0x00080000,
        //Internal state attributes
        //These are used to check if the more time consuming attributes need to be enumerated.
        PEENUMERATED =            0x00100000,
        PECHKSUM =                0x00200000,
        SIGENUMERATED =            0x00400000,
        VERSIONINFOCHECKED =    0x01000000,
        //Looks like I had to add another executable attribute
        PEPLUS =                0x02000000
    };

    mutable DWORD bits; //Container for the bits in the enum above
                                      //This is mutable because constant functions
                                      //use the bitset to cache their responses

    //Filename
    std::wstring fileName;
    
    //PE Information
    mutable FILETIME headerTime;
    mutable DWORD headerSum;
    mutable DWORD calcSum;

    //Version information block
    mutable std::vector<BYTE> versionInformationBlock;
    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;
        bool operator==(LANGANDCODEPAGE &rhs)
        {
            return wLanguage == rhs.wLanguage && wCodePage == rhs.wCodePage;
        };
        bool operator<(LANGANDCODEPAGE &rhs)
        {
            if (wLanguage != rhs.wLanguage)
                return wLanguage < rhs.wLanguage;
            return wCodePage < rhs.wCodePage;
        };
    };
    mutable std::vector<LANGANDCODEPAGE> versionTranslations;

    //Enumeration functions
    //When the results aren't cached in the bitset bits, these functions calculate
    //the correct values and place them into the bitset.
    void initPortableExecutable() const;
    void sigVerify() const;
    void enumVersionInformationBlock() const;

    //Group set functions
    //These functions set a large number of items according to an external data structure
    void setAttributesAccordingToDWORD(DWORD win32Attribs) const;

    //Internal calculation functions
    void inline appendAttributeCharacter(std::wstring &result, const TCHAR attributeCharacter, const size_t curBit) const;
    std::wstring getVersionInformationString(const std::wstring&) const;
    template <typename hashType> std::wstring getHash() const;

    //PE Checksum functions (from Code Project)
    WORD ChkSum(WORD oldChk, USHORT * ptr, DWORD len) const;
    DWORD GetPEChkSum(LPCTSTR filename) const;

    //SFC Safe Mode Fix functions
    static std::vector<std::wstring> sfcFileStrings;
    enum {
        NOT_CHECKED,
        NO_SFCFILES_DLL,
        ENUMERATED
    } SFCStates;
    static unsigned int sfcState;
    void buildSfcList() const;

    inline void setupWin32Attributes() const;
public:
    //Returns the Win32 handle for the file
    HANDLE getFileHandle(bool readOnly = true) const;
    //Construct a fileData record using a Win32FindData structure and a search path.
    FileData(const WIN32_FIND_DATA &rawData, const std::wstring& root);
    //Construct a fileData record using a raw filename
    FileData(const std::wstring &fileNameBuild);

    //Type extensions
    //Used for comparisons and for getting implicit conversions
    bool operator<(FileData& rhs);
    operator std::wstring () const {return getFileName(); };

    //Accessor methods
    //Return information about the current filedata object

    //Size
    inline unsigned __int64 getSize() const;

    //Filename
    inline const std::wstring & getFileName() const;

    //Access times
    inline const WIN32_FILE_ATTRIBUTE_DATA getAttributeData() const;
    inline const FILETIME getLastAccessTime() const;
    inline const FILETIME getLastModTime() const;
    inline const FILETIME getCreationTime() const;
    inline const FILETIME getPEHeaderTime() const;

    //Attributes
    //VFIND style attribute string
    std::wstring getAttributesString() const;
    //PE Attribute string
    std::wstring getPEAttribsString() const;
    //Basic WIN32 Attributes
    inline bool isArchive() const;
    inline bool isCompressed() const;
    inline bool isDirectory() const;
    inline bool isFile() const;
    inline bool isHidden() const;
    inline bool isReadOnly() const;
    inline bool isSystem() const;
    inline bool isVolumeLabel() const;
    inline bool isWritable() const;
    inline bool isTemporary() const;
    inline bool isReparsePoint() const;
    //PE Data Attributes
    inline bool isPE() const;
    inline bool isNE() const;
    inline bool isLE() const;
    inline bool isMZ() const;
    inline bool isPEPlus() const;
    inline bool isStrongExecutable() const;
    inline bool peHeaderChecksumIsValid() const;
    inline bool isDLL() const;
    inline DWORD getPEHeaderCheckSum() const;
    inline DWORD getPECalculatedCheckSum() const;
    void resetPEHeaderCheckSum();
    inline bool peHeaderTimeIsValid() const;
    //Digital Signature Attributes
    inline bool hasValidDigitalSignature() const;
    //Windows File Protection Attributes
    bool isSfcProtected() const;

    //Hashing functions
    std::wstring MD5() const;
    std::wstring SHA1() const;
    std::wstring SHA224() const;
    std::wstring SHA256() const;
    std::wstring SHA384() const;
    std::wstring SHA512() const;

    // Version information functions
    inline std::wstring GetVerCompany() const;
    inline std::wstring GetVerDescription() const;
    inline std::wstring GetVerVersion() const;
    inline std::wstring GetVerProductName() const;
    inline std::wstring GetVerCopyright() const;
    inline std::wstring GetVerOriginalFileName() const;
    inline std::wstring GetVerTrademark() const;
    inline std::wstring GetVerInternalName() const;
    inline std::wstring GetVerComments() const;
    inline std::wstring GetVerPrivateBuild() const;
    inline std::wstring GetVerSpecialBuild() const;
    
    // Logging function
    void write();
};

//
// Inline implementations
//
inline unsigned __int64 FileData::getSize() const
{
    __int64 result;
    WIN32_FILE_ATTRIBUTE_DATA attributeData;
    if(!GetFileAttributesEx(fileName.c_str(), GetFileExInfoStandard, &attributeData))
        return 0;
    if (attributeData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return 0;
    result = attributeData.nFileSizeHigh;
    result = result << 32;
    result |= attributeData.nFileSizeLow;
    return result;
}
inline const std::wstring & FileData::getFileName() const
{
    return fileName;
}

inline const WIN32_FILE_ATTRIBUTE_DATA FileData::getAttributeData() const
{
    disable64.disableFS();
    WIN32_FILE_ATTRIBUTE_DATA attributeData;
    if(GetFileAttributesEx(fileName.c_str(), GetFileExInfoStandard, &attributeData) == 0)
    {
        ZeroMemory(&attributeData, sizeof(attributeData));
    }
    disable64.enableFS();
    return attributeData;
}

inline const FILETIME FileData::getLastAccessTime() const
{
    return getAttributeData().ftLastAccessTime;
}
inline const FILETIME FileData::getLastModTime() const
{
    return getAttributeData().ftLastWriteTime;
}
inline const FILETIME FileData::getCreationTime() const
{
    return getAttributeData().ftCreationTime;
}
inline const FILETIME FileData::getPEHeaderTime() const
{
    initPortableExecutable();
    if (!(bits & ISPE))
    {
        FILETIME zero;
        ZeroMemory(&zero, sizeof(zero));
        return zero;
    }
    return headerTime;
}

inline bool FileData::isArchive() const
{
    setupWin32Attributes();
    return (bits & ARCHIVE) != 0;
}

inline bool FileData::isCompressed() const
{
    setupWin32Attributes();
    return (bits & COMPRESSED) != 0;
}

inline bool FileData::isDirectory() const
{
    setupWin32Attributes();
    return (bits & DIRECTORY) != 0;
}

inline bool FileData::isFile() const
{
    setupWin32Attributes();
    return (bits & FILE) != 0;
}

inline bool FileData::isHidden() const
{
    setupWin32Attributes();
    return (bits & HIDDEN) != 0;
}

inline bool FileData::isReadOnly() const
{
    setupWin32Attributes();
    return (bits & READONLY) != 0;
}

inline bool FileData::isSystem() const
{
    setupWin32Attributes();
    return (bits & SYSTEM) != 0;
}

inline bool FileData::isVolumeLabel() const
{
    setupWin32Attributes();
    return (bits & VOLLABEL) != 0;
}

inline bool FileData::isWritable() const
{
    setupWin32Attributes();
    return (bits & WRITABLE) != 0;
}

inline bool FileData::isTemporary() const
{
    setupWin32Attributes();
    return (bits & TEMPORARY) != 0;
}

inline bool FileData::isReparsePoint() const
{
    setupWin32Attributes();
    return (bits & REPARSE) != 0;
}

inline bool FileData::isPE() const
{
    initPortableExecutable();
    return (bits & ISPE) != 0;
}

inline bool FileData::isPEPlus() const
{
    initPortableExecutable();
    return (bits & PEPLUS) != 0;
}

inline bool FileData::isNE() const
{
    initPortableExecutable();
    return (bits & ISNE) != 0;
}

inline bool FileData::isLE() const
{
    initPortableExecutable();
    return (bits & ISLE) != 0;
}

inline bool FileData::isMZ() const
{
    initPortableExecutable();
    return (bits & ISMZ) != 0;
}

inline bool FileData::isStrongExecutable() const
{
    return isPE() || isNE() || isLE();
}

inline bool FileData::hasValidDigitalSignature() const
{
    if (!isMZ())
        return false;
    if (!(bits & SIGENUMERATED))
        sigVerify();
    return (bits & SIGVALID) != 0;
}

inline bool FileData::peHeaderChecksumIsValid() const
{
    initPortableExecutable();
    if (headerSum == 0)
        return true;
    return headerSum == getPECalculatedCheckSum();
}

inline bool FileData::isDLL() const
{
    initPortableExecutable();
    return (bits & DLL) != 0;
}

inline DWORD FileData::getPEHeaderCheckSum() const
{
    initPortableExecutable();
    if (!(bits & ISPE))
        return 0;
    return headerSum;
}

inline DWORD FileData::getPECalculatedCheckSum() const
{
    initPortableExecutable();
    if (!(bits & ISPE))
        return 0;
    if (!(bits & PECHKSUM))
        calcSum = GetPEChkSum(fileName.c_str());
    return calcSum;
}

inline bool FileData::peHeaderTimeIsValid() const
{
    SYSTEMTIME curTimeSys;
    GetLocalTime(&curTimeSys);
    FILETIME curTime;
    SystemTimeToFileTime(&curTimeSys,&curTime);
    //One day padding ... one day in 100 nanosecond intervals is 864000000000
    return getPEHeaderTime() < (getLastModTime() + 864000000000ull) && getPEHeaderTime() < (curTime + 864000000000ull);
}

inline std::wstring FileData::GetVerCompany() const
{
    return getVersionInformationString(L"CompanyName");
}
inline std::wstring FileData::GetVerDescription() const
{
    return getVersionInformationString(L"FileDescription");
}
inline std::wstring FileData::GetVerVersion() const
{
    return getVersionInformationString(L"FileVersion");
}
inline std::wstring FileData::GetVerProductName() const
{
    return getVersionInformationString(L"ProductName");
}
inline std::wstring FileData::GetVerCopyright() const
{
    return getVersionInformationString(L"LegalCopyright");
}
inline std::wstring FileData::GetVerOriginalFileName() const
{
    return getVersionInformationString(L"OriginalFilename");
}
inline std::wstring FileData::GetVerTrademark() const
{
    return getVersionInformationString(L"LegalTrademarks");
}
inline std::wstring FileData::GetVerInternalName() const
{
    return getVersionInformationString(L"InternalName");
}
inline std::wstring FileData::GetVerComments() const
{
    return getVersionInformationString(L"Comments");
}
inline std::wstring FileData::GetVerPrivateBuild() const
{
    return getVersionInformationString(L"PrivateBuild");
}
inline std::wstring FileData::GetVerSpecialBuild() const
{
    return getVersionInformationString(L"SpecialBuild");
}

inline std::wstring GetHashErrorMessage(DWORD error)
{
    if (error == ERROR_LOCK_VIOLATION)
    {
        return std::wstring(L"!HASH: ERROR_LOCK_VIOLATION !!!!", 32);
    }
    else
    {
        wchar_t buff[33];
        _snwprintf_s(buff, 32, L"!HASH: WIN32 ERROR 0x%08X !!", error);
        return std::wstring(buff, 32);
    }
}

template <typename hashType> 
std::wstring FileData::getHash() const
{
    using std::swap;
    static const wchar_t constantHexArray[] = L"0123456789ABCDEF";

    OVERLAPPED overlappedIoBlock = {};
    overlappedIoBlock.hEvent = ::CreateEventW(nullptr, false, false, nullptr);
    HANDLE file;
    disable64.disableFS();
    file = ::CreateFileW(getFileName().c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED, nullptr);
    DWORD error = GetLastError();
    disable64.enableFS();
    if (file == INVALID_HANDLE_VALUE)
    {
        return GetHashErrorMessage(::GetLastError());
    }

    hashType hash;
    typedef unsigned char byte;
    auto const bytesToAttempt = 1024 * 4; //4k
    std::unique_ptr<byte[]> buffer1(new byte[bytesToAttempt]);
    std::unique_ptr<byte[]> buffer2(new byte[bytesToAttempt]);
    byte* readingBuffer = buffer1.get();
    byte* hashingBuffer = buffer2.get();

    if (::ReadFile(file, readingBuffer, bytesToAttempt, nullptr, &overlappedIoBlock) == 0)
    {
    	DWORD error = ::GetLastError();
        if (error != ERROR_IO_PENDING)
        {
            return GetHashErrorMessage(error);
        }
    }

    for (;;)
    {
        DWORD bytesRead;
        if (::GetOverlappedResult(file, &overlappedIoBlock, &bytesRead, true) == 0)
        {
            DWORD error = GetLastError();
            if (error == ERROR_HANDLE_EOF)
            {
                break;
            }
            else if (error != 0)
            {
                CloseHandle(file);
                CloseHandle(overlappedIoBlock.hEvent);
                return GetHashErrorMessage(error);
            }
        }

        // Swap in the new buffer for reading
        swap(readingBuffer, hashingBuffer);
        // Fire off a new read call
        auto lastOffset = reinterpret_cast<std::uint64_t>(overlappedIoBlock.Pointer);
        lastOffset += bytesRead;
        overlappedIoBlock.Pointer = reinterpret_cast<PVOID>(lastOffset);
        if (::ReadFile(file, readingBuffer, bytesToAttempt, nullptr, &overlappedIoBlock) == 0)
        {
            DWORD error = ::GetLastError();
            if (error != ERROR_IO_PENDING)
            {
                CloseHandle(file);
                CloseHandle(overlappedIoBlock.hEvent);
                return GetHashErrorMessage(error);
            }
        }

        // Hash what we just read
        hash.Update(hashingBuffer,bytesRead);
    }

    CloseHandle(file);
    CloseHandle(overlappedIoBlock.hEvent);

    std::unique_ptr<byte[]> rawHash(new byte[hash.DigestSize()]);
    hash.Final(rawHash.get());

    std::wstring result;
    result.resize(hash.DigestSize() * 2);
    DWORD len = hash.DigestSize();
    for (unsigned short int idx = 0; idx < len; idx++)
    {
        result[(len*2-1)-2*idx] = constantHexArray[(rawHash[(len-1)-idx] & 0x0F)];
        result[(len*2-1)-(2*idx+1)] = constantHexArray[(rawHash[(len-1)-idx] & 0xF0) >> 4];
    };
    return result;
}

inline void FileData::setupWin32Attributes() const
{
    if (bits & WIN32ENUMD) return;
    setAttributesAccordingToDWORD(GetFileAttributes(fileName.c_str()));
}
