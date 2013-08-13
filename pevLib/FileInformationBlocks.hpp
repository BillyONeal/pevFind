//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <utility>

namespace pevFind
{
enum class FileDataType : std::uint16_t
{
    FindData,
    Name,
    ShortName,
    Attributes,
    PortableExecutableInformation,
    SignatureInformation,
    Md5Information,
    Sha1Information,
    Sha224Information,
    Sha256Information,
    Sha384Information,
    Sha512Information,
    VersionInformation
};

class FileInformationBlock;

class FileInformationBlockDefinition
{
    std::vector<std::pair<FileDataType, std::uint16_t>> indicies;
    bool defined;
public:
    void DefineData(FileDataType type, std::uint16_t size);
    void LockDefinition() throw();
    std::unique_ptr<FileInformationBlock> CreateBlock() const;
    void* GetData(FileInformationBlock* block, FileDataType dataType) const throw();
};

}
