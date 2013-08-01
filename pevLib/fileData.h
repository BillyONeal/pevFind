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
#pragma warning(push)
#pragma warning(disable: 4512)
#pragma warning(disable: 4100)
#pragma warning(disable: 4244)
#pragma warning(disable: 4127)
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp562/md5.h>
#include <cryptopp562/sha.h>
#pragma warning(pop)
#include "utility.h"
#include "../LogCommon/Win32Glue.hpp"

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
        SIGPRESENT =             0x00001000,
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
    Instalog::UniqueHandle getFileHandle(bool readOnly = true) const;
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
    inline bool hasAuthenticodeSignature() const;
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

inline bool FileData::hasAuthenticodeSignature() const
{
    initPortableExecutable();
    return (bits & SIGPRESENT) != 0;
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

inline void FileData::setupWin32Attributes() const
{
    if (bits & WIN32ENUMD) return;
    setAttributesAccordingToDWORD(GetFileAttributes(fileName.c_str()));
}
