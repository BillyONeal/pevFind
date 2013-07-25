//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <string>
#include <map>
#include <cstdint>
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

class LoadedFile
{
    SourceLocation startLocation;
    std::uint32_t length;
    std::wstring input;
    std::wstring name;
public:
    LoadedFile(SourceLocation startLocation_, std::uint32_t length_, std::wstring input_, std::wstring name_) throw()
        : startLocation(startLocation_)
        , length(length_)
        , input(std::move(input_))
        , name(std::move(name_))
    { }
    LoadedFile(LoadedFile&& other) throw()
        : startLocation(other.startLocation)
        , length(other.length)
        , input(std::move(other.input))
        , name(std::move(other.name))
    {
    }
    LoadedFile& operator=(LoadedFile&& other) throw()
    {
        startLocation = other.startLocation;
        length = other.length;
        input = std::move(other.input);
        name = std::move(other.name);
    }
    SourceLocation GetStartLocation() const throw() { return startLocation; }
    std::uint32_t GetLength() const throw() { return length; }
};

class SourceMananger
{
    std::vector<LoadedFile> loadedFiles;
public:
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
    std::wstring const& GetLine() const;

    /**
     * Determines whether the load succeeded.
     * 
     * @return true if the line was loaded successfully; otherwise, false.
     */
    bool Success() const;

    /**
     * Gets the error string on error.
     * @return The error string if the load was successful, or the empty string if the load was successful.
     */
    std::wstring const& GetError() const;
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
    virtual ~ILoadLineResolver();
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
    void Add(std::wstring&& name, std::wstring&& value);

    /**
     * Loads the given named object as a command line.
     * @param name The name to load.
     * @return The result of the load attempt.
     */
    virtual LoadLineResult LoadLineByName(std::wstring const& name) const override;
};

class LexicalAnalyzer
{
public:
    LexicalAnalyzer(std::unique_ptr<ILoadLineResolver> resolver);
    bool ParseCommandLine(std::wstring commandLineToParse);
    std::vector<Token> const& GetTokenStream() const;
private:
    std::vector<Token> tokenStream;
};

}
