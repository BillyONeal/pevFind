//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <string>
#include <map>
#include <cstdint>
#include <memory>
#include <ostream>
#include "Logic.hpp"

namespace pevFind
{
/**
 * Values that represent token types.
 * 
 * All the values except Condition and Setting are the same as their literal text would suggest.
 * Condition are arguments that result in conditions placed on the object PEV is parsing.
 * Setting are arguments that change a setting, but don't get placed into the tree. (e.g.
 * -md5#123# results in a Condition, while --c#123# results in a Setting)
 */
enum class TokenType
{
    ///< An enum constant representing the bracket option ( { )
    Bracket,
    ///< An enum constant representing the end bracket option ( } )
    EndBracket,
    ///< An enum constant representing the and option ( AND )
    And,
    ///< An enum constant representing the or option ( OR )
    Or,
    ///< An enum constant representing the exclusive-or option ( XOR )
    Xor,
    ///< An enum constant representing if option ( IF )
    If,
    ///< An enum constant representing the else option ( ELSE )
    Else,
    ///< An enum constant representing the then option ( THEN )
    Then,
    ///< An enum constant representing a condition option ( -Xxx )
    Condition,
    ///< An enum constant representing a setting option ( -Xxx )
    Setting
};

/**
 * An index into the logical command line input.
 */
typedef std::uint32_t SourceLocation;

/**
 * Loaded file record.
 */
class LoadedFile
{
    std::wstring name;
    SourceLocation startLocation;
    SourceLocation length;
public:

    /**
     * Initializes a new instance of the LoadedFile class.
     * @param startLocation_ The location where this file locally starts.
     * @param length_        The length of this file's content.
     * @param name_          The name of this file.
     */
    LoadedFile(SourceLocation startLocation_, SourceLocation length_, std::wstring name_) throw();

    /**
     * Move constructor.
     * @param [in,out] other The instance to move.
     */
    LoadedFile(LoadedFile&& other) throw();

    /**
     * Move assignment operator.
     * @param [in,out] other The instance to move.
     * @return A shallow copy of this instance.
     */
    LoadedFile& operator=(LoadedFile&& other) throw();

    /**
     * Gets the location where this file locally starts.
     * @return The location where this file locally starts.
     */
    SourceLocation GetStartLocation() const throw();

    /**
     * Gets the length of this file.
     * @return The length of this file.
     */
    SourceLocation GetLength() const throw();
};

/**
 * Manages the logical input stream. Allows logical replacement of the input stream (for
 * implementing --loadline)
 */
class SourceManager
{
    std::vector<LoadedFile> loadedFiles;
    std::wstring backingBuffer;
public:

    /**
     * Initializes a new instance of the SourceManager class.
     * @param startContent The starting content; typically the input command line.
     * @param startName    (Optional) The name of the starting content. Defaults to L"Command Line"
     */
    SourceManager(std::wstring startContent, std::wstring startName = L"Command Line");

    /**
     * Logically installs a file and replaces a portion of the input.
     * @param insertionLocation The insertion location which the inserted content replaces.
     * @param replaceLength     The length of the insertion location which is replaced.
     * @param content           The content placed in the replaced region.
     * @param name              The name of the content.
     */
    void InstallFile(SourceLocation insertionLocation, SourceLocation replaceLength, std::wstring const& content, std::wstring name);

    /**
     * Array indexer operator.
     * @param location The location to retrieve.
     * @return The value at the logical index provided.
     */
    wchar_t operator[](SourceLocation location) const throw();

    /**
     * Gets the logical string.
     * 
     * This member function is intended for debugging and testing purposes only.
     * 
     * @return The logical string.
     */
    std::wstring const& GetLogicalString() const throw() { return backingBuffer; }

     /**
     * Generates a source listing for the whole logical command line.
     * 
     * A source listing is a listing of the location from which a given piece of text comes in the
     * logical input. For instance, given the following:
     * @code
     *      pevFind.exe foo --loadline#example# bar
     *      
     *      example contains:
     *      baz
     *      Logical input is:
     *      pevFind.exe foo baz bar
     * @endcode
     * the output would end up being:
     * @code
     *      |- Command Line-------|
     *                      |2|
     *      2: example
     * @endcode
     * 
     * @return The source listing.
     */
    std::wstring GenerateSourceListing() const { return GenerateSourceListing(0, static_cast<SourceLocation>(backingBuffer.size())); }

    /**
     * Generates a source listing.
     * 
     * A source listing is a listing of the location from which a given piece of text comes in the
     * logical input. For instance, given the following:
     * @code
     *      pevFind.exe foo --loadline#example# bar
     *      
     *      example contains:
     *      baz
     *      Logical input is:
     *      pevFind.exe foo baz bar
     * @endcode
     * the output would end up being:
     * @code
     *      |- Command Line-------|
     *                      |2|
     *      2: example
     * @endcode
     * 
     * @param startLocation The start of the range to list.
     * @param endLocation   The end of the range to list.
     * @return The source listing.
     */
    std::wstring GenerateSourceListing(SourceLocation startLocation, SourceLocation endLocation) const;
};

/**
 * Load line result.
 */
class LoadLineResult
{
    std::wstring lineOrError;
    bool success;
    LoadLineResult(std::wstring&& lineOrError_, bool success_);
public:
    /**
     * Creates a LoadLineResult from the given command line.
     * @param [in,out] line The line result.
     * @return A new "success" LoadLineResult containing @a line.
     */
    static LoadLineResult FromLineValue(std::wstring line);

    /**
     * Creates a LoadLineResult from the given failure message.
     * @param [in,out] error The error message to report.
     * @return A new "failure" LoadLineResult containing @a error.
     */
    static LoadLineResult FromFailure(std::wstring error);

    /**
     * Gets the line that was loaded.
     * @return The line if load was successful, or the empty string if the load was unsuccessful.
     */
    std::wstring const& GetLine() const throw();

    /**
     * Steals the line that was loaded.
     * 
     * Preconditions: Success()
     * After this member function returns, this LoadLineResult is in an undefined state.
     * 
     * @return The loaded line.
     */
    std::wstring&& StealLine() throw();

    /**
     * Determines whether the load succeeded.
     * 
     * @return true if the line was loaded successfully; otherwise, false.
     */
    bool Success() const throw();

    /**
     * Gets the error string on error.
     * @return The error string if the load was successful, or the empty string if the load was successful.
     */
    std::wstring const& GetError() const throw();
};

/**
 * Load line resolver interface. Provides the --loadline feature support, and allows
 * overriding this behavior to be entirely in memory if desired.
 */
struct ILoadLineResolver
{
    /**
     * Loads the given named object as a command line.
     * @param name The name to load.
     * @return The result of the load attempt.
     */
    virtual LoadLineResult LoadLineByName(std::wstring const& name) const = 0;
    virtual ~ILoadLineResolver() throw();
};

/**
 * File load line resolver. Loads the line from a file from disk.
 * @sa ILoadLineResolver
 */
struct FileLoadLineResolver : public ILoadLineResolver
{
    /**
     * Loads the given named object as a command line.
     * @param name The name to load.
     * @return The result of the load attempt.
     */
    virtual LoadLineResult LoadLineByName(std::wstring const& name) const override;
};

/**
 * Preconfigured load line resolver. Loads the line from a preconfigured value.
 * @sa ILoadLineResolver
 */
class PreconfiguredLoadLineResolver : public ILoadLineResolver
{
    std::map<std::wstring, std::wstring> lines;
public:
    /**
     * Adds the given preconfigured --loadline argument.
     * @param [in,out] name  The name to register.
     * @param [in,out] value The value of the line to associate.
     */
    void Add(wchar_t const* name, wchar_t const* value);

    /**
     * Loads the given named object as a command line.
     * @param name The name to load.
     * @return The result of the load attempt.
     */
    virtual LoadLineResult LoadLineByName(std::wstring const& name) const override;
};

}
