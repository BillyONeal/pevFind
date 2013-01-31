#ifndef _REGISTRY_H_INCLUDED
#define _REGISTRY_H_INCLUDED
//          Copyright Billy O'Neal 2012
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// registry.h -- Defines common facilities used for registry access, as well
// as some helper functions for all registry subprograms.

#include <vector>
#include <string>
#include <exception>
#include <windows.h>

namespace registry
{
    class registryValue;
    /*
     * Key - This is the primary class with regard to the registry handling
     * functions. When you instantiate a key, it can be compared to opening
     * a handle to the key in question when dealing with the C API.
     *
     * The handle itself is instantiated as a part of the class, and is
     * freed automaticly when the class destroys itself.
     *
     * Currently, this library uses the DuplicateHandle function to
     * maintain the refrence count to the HREG in question. Faster
     * alternateives such as a manually managed refrence count are possible,
     * but are not implemeneted at present time.
     */
    class registryKey
    {
        friend class registryValue;
        HKEY hWin32;
        HKEY root;
        std::wstring path_;
    public:
        registryKey(const std::wstring& path);
        registryKey(const registryKey& toCopy);
        ~registryKey();
        registryKey& operator=(const registryKey& lhs);
        registryValue operator[](const std::wstring& lhs);
    };

    /*
     * Value - This class allows one to access the data contained within a
     * key object. The value object exists as sort of an iterator, in that
     * it is valid only as long as the key that created it exists.
     *
     * Attempting to use a Value object after it's parent key has been
     * destroyed results in an access violation.
     */
    class registryValue
    {
        friend class registryKey;
        registryKey * parent_;
        std::wstring name_;
        registryValue(registryKey& parent, const std::wstring& name);
        void constructDataBlock(DWORD * type, std::vector<char>& dataBlock) const;
        std::wstring displayString(const std::vector<char> &dataBlock) const;
        std::wstring displayHex(const std::vector<char> &dataBlock, DWORD type);
    public:
        registryValue(const registryValue& rhs);
        registryValue& operator=(const registryValue& rhs);
        DWORD getType();
        std::vector<char> getRawDataBlock();
        const std::wstring& getName();
        registryKey& getParent();

        //std::wstring conversions.
        std::wstring asString();
        void setString(const std::wstring& toSet);
        std::wstring operator()();
        registryValue& operator=(const std::wstring& rhs);

        //DWORD conversions.
        bool asDword(DWORD * result);
        void setDword(DWORD toSet);

        //QWORD conversions.
        bool asQword(__int64 * result);
        void setQword(__int64 toSet);
    };

    /*
     * RegistryException - Thrown when there is an error accessing the
     * registry.
     */
    struct registryException : public std::runtime_error
    {
        explicit registryException(const std::string &what_arg) : std::runtime_error(what_arg) {};
    };

    /*
     * The following functions are for internal use of the library and
     * are not meant for public consumption.
     */
    
    std::wstring::const_iterator getKeyRoot(std::wstring::const_iterator begin,
                                            std::wstring::const_iterator end,
                                            HKEY &result);
};

#endif _REGISTRY_H_INCLUDED
