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
 * An index into the logical command line input.
 */
typedef std::uint32_t SourceLocation;

/**
 * Case insensitive character constant.
 */
class CaseInsensitiveConstant
{
    std::unique_ptr<wchar_t[]> buffer;
    SourceLocation size_;
    wchar_t* GetLowerCaseBegin() const throw();
    wchar_t* GetLowerCaseEnd() const throw();
    wchar_t* GetUpperCaseBegin() const throw();
    wchar_t* GetUpperCaseEnd() const throw();
public:
    typedef wchar_t const* const_iterator;

    /**
     * Initializes a new instance of the CaseInsensitiveConstant class containing the empty string.
     */
    CaseInsensitiveConstant();

    /**
     * Initializes a new instance of the CaseInsensitiveConstant class containing the specified
     * null terminated string.
     * @param value The null terminated string to which this instance is set.
     */
    explicit CaseInsensitiveConstant(wchar_t const* value);

    /**
     * Initializes a new instance of the CaseInsensitiveConstant class containing the specified
     * standard string.
     * @param value The standard string to which this instance is set.
     */
    explicit CaseInsensitiveConstant(std::wstring const& value);

    /**
     * Initializes a new instance of the CaseInsensitiveConstant class using the indicated length specified string.
     * @param value The value to which this instance is set.
     * @param valueSize The number of characters in @a value, excluding the terminating null character (if any)
     */
    CaseInsensitiveConstant(wchar_t const* value, SourceLocation valueSize);

    /**
     * Initializes a new copy of the CaseInsensitiveConstant class.
     * @param toCopy to copy.
     */
    CaseInsensitiveConstant(CaseInsensitiveConstant const& toCopy);

    /**
     * Move initializes the CaseInsensitiveConstant class.
     * @param toMove to move.
     */
    CaseInsensitiveConstant(CaseInsensitiveConstant&& toMove) throw();

    /**
     * Sets this instance to the given null terminated string.
     * @param value The null terminated string to which this instance is set.
     * @return *this
     */
    CaseInsensitiveConstant& assign(wchar_t const* value);

    /**
     * Sets this instance to the given standard string.
     * @param value The string to which this instance is set.
     * @return *this
     */
    CaseInsensitiveConstant& assign(std::wstring const& value);

    /**
     * Sets this instance to the given length specified string.
     * @param value The value to which this instance will be set.
     * @param valueSize The number of characters in @a value, excluding the terminating null character (if any)
     * @return *this
     */
    CaseInsensitiveConstant& assign(wchar_t const* value, SourceLocation valueSize);

    /**
     * Sets this instance to a copy of the instance indicated.
     * @param toCopy The CaseInsensitiveConstant to copy.
     * @return *this
     */
    CaseInsensitiveConstant& assign(CaseInsensitiveConstant const& toCopy);

    /**
     * Sets this instance to the instance indicated.
     * @param toCopy The CaseInsensitiveConstant to move.
     * @return *this
     */
    CaseInsensitiveConstant& assign(CaseInsensitiveConstant&& toMove) throw();

    /**
     * Assignment operator for null terminated strings.
     * @param value The null terminated string to which this instance is assigned.
     * @return *this
     */
    CaseInsensitiveConstant& operator=(wchar_t const* value);

    /**
     * Assignment operator for strings.
     * @param value The standard string to which this instance is assigned.
     * @return *this
     */
    CaseInsensitiveConstant& operator=(std::wstring const& value);

    /**
     * Copy assignment operator.
     * @param toCopy The instance to which this instance's state is set.
     * @return *this
     */
    CaseInsensitiveConstant& operator=(CaseInsensitiveConstant const& toCopy);

    /**
     * Move assignment operator.
     * @param toMove The instance to move.
     * @return *this
     */
    CaseInsensitiveConstant& operator=(CaseInsensitiveConstant&& toMove) throw();

    /**
     * Gets the size of this instance, in characters.
     * @return The size of this instance, in characters.
     */
    SourceLocation size() const throw();

    /**
     * Gets the lower case version of this instance as a null terminated string.
     * @return The lower case version of this instance as a null terminated string.
     */
    wchar_t const* lower_cstr() const throw();

    /**
     * Gets the upper case version of this instance as a null terminated string.
     * @return The upper case version of this instance as a null terminated string.
     */
    wchar_t const* upper_cstr() const throw();

    /**
     * Gets the begin iterator for the lower case view of this instance.
     * @return The begin iterator for the lower case view of this instance.
     */
    wchar_t const* lcbegin() const throw();

    /**
     * Gets the end iterator for the lower case view of this instance.
     * @return The end iterator for the lower case view of this instance.
     */
    wchar_t const* lcend() const throw();

    /**
     * Gets the begin iterator for the upper case view of this instance.
     * @return The begin iterator for the upper case view of this instance.
     */
    wchar_t const* ucbegin() const throw();

    /**
     * Gets the end iterator for the upper case view of this instance.
     * @return The end iterator for the upper case view of this instance.
     */
    wchar_t const* ucend() const throw();

    /**
     * Swaps this instance with the instance given.
     * @param other The instance with which this instance is swapped.
     */
    void swap(CaseInsensitiveConstant& other) throw();
};

/**
 * Swaps a pair of CaseInsensitiveConstant instances.
 * @param lhs The left hand side.
 * @param rhs The right hand side.
 */
void swap(CaseInsensitiveConstant& lhs, CaseInsensitiveConstant& rhs) throw();

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

    /**
     * Gets logical end of this file. Equivalent to GetStartLocation() + GetLength().
     * @return The logical end of this file.
     */
    SourceLocation GetLogicalEnd() const throw() { return GetStartLocation() + GetLength(); }

    /**
     * Gets the name of this file.
     * @return The name of this file.
     */
    std::wstring const& GetName() const throw() { return name; }

    /**
     * Adds the given amount of length to this LoadedFile.
     * @return .
     */
    void AddLength(std::int32_t addedLength) throw();
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
     * Gets the number of characters in the logical buffer.
     * @return The number of characters in the logical buffer.
     */
    SourceLocation size() const throw() { return static_cast<SourceLocation>(backingBuffer.size()); }

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

    /**
     * Queries if the given constant at the given location.
     * @param location The location to start searching for the constant.
     * @param literalValue A string constant to check for.
     * @return true if the value @a literalValue is at @a location in the logical text stream.
     */
    bool ConstantAt(SourceLocation location, std::wstring const& literalValue) const throw();

    /**
     * Queries if the given constant at the given location.
     * @param location The location to start searching for the constant.
     * @param caseInsensitiveValue A case insensitive string constant to check for. A character
     *  match is one which matches either the lower case view or the upper case view of the
     *  constant.
     * @return true if the value @a caseInsensitiveValue is at @a location in the logical text
     *  stream.
     */
    bool ConstantAt(SourceLocation location, CaseInsensitiveConstant const& caseInsensitiveValue) const throw();

    /**
     * Searches for the next predicate match after the given index.
     * @tparam typename Functor Type of the functor.
     * @param startAt The search start location.
     * @param predicate The predicate to match.
     * @return The location of the first match of @a predicate found after @a startAt, or size()
     *  if no character matched.
     */
    template <typename Functor>
    SourceLocation FindNextPredicateMatchAfter(SourceLocation startAt, Functor predicate) const throw()
    {
        auto const maxLocation = static_cast<SourceLocation>(this->backingBuffer.size());
        startAt = std::min(maxLocation, startAt);
        for (; startAt < maxLocation; ++startAt)
        {
            if (predicate(this->backingBuffer[startAt]))
            {
                break;
            }
        }

        return startAt;
    }

    /**
     * Searches for the next instance of a character after the given SourceLocation.
     * @param startAt The search start location.
     * @param characterToSearchFor The character to search for.
     * @return The location of the found character, or size() if it was not found.
     */
    SourceLocation FindNextCharacterAfter(SourceLocation startAt, wchar_t characterToSearchFor) const throw()
    {
        return FindNextPredicateMatchAfter(startAt, [=](wchar_t candidate) { return candidate == characterToSearchFor; });
    }

    /**
     * Generates a string for the logical range [begin, end)
     * @param begin The begin index of the string to generate.
     * @param end The end index of the string to generate.
     * @return A string comprising the characters in the range [begin, end)
     */
    std::wstring StringForRange(SourceLocation begin, SourceLocation end) const;
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

enum class ArgumentType
{
    Uninitialized,
    Literal,
    Quoted,
    Dashed
};

class LexicalAnalyzer : boost::noncopyable
{
    std::unique_ptr<ILoadLineResolver> loadLineResolver;
    SourceManager sm;
    SourceLocation lexicalStart;
    SourceLocation lexicalEnd;
    SourceLocation argumentStart;
    SourceLocation argumentEnd;
    SourceLocation parameterStart;
    SourceLocation parameterEnd;
    ArgumentType argumentType;
    std::wstring errorMessage;
public:
    LexicalAnalyzer(std::unique_ptr<ILoadLineResolver> loadLineResolver_, std::wstring inputString);
    std::wstring GetLexicalTokenRaw() const;
    std::wstring GetLexicalTokenArgument() const;
    std::wstring GetLexicalTokenParameter() const;
    std::wstring const& GetErrorMessage() const throw();
    bool IsDashedArgument() const throw();
    bool NextLexicalToken();
};

}
