//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <string>
#include <map>
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

class Token
{
public:
    typedef std::wstring::const_iterator const_iterator;

    /**
     * Initializes a new instance of the Token class.
     * @param type_ The type of token. Must not be TokenType::Condition.
     * @param sourceBegin_ The begin iterator pointing to the first character making up this token.
     * @param sourceEnd_ The end iterator pointing to one past the last character making up this token.
     * @param containingBegin_ The begin iterator pointing to the first character of the logical argument containing this token.
     * @param containingEnd_ The end iterator pointing to the last character of the logical argument containing this token.
     */
    Token(TokenType type_, const_iterator sourceBegin_, const_iterator sourceEnd_, const_iterator containingBegin_, const_iterator containingEnd_);

    /**
     * Initializes a new instance of the Token class with the given logical node. The type of the constructed token is TokenType::Condition.
     * @param type_ The type of token. Must not be TokenType::Condition.
     * @param sourceBegin_ The begin iterator pointing to the first character making up this token.
     * @param sourceEnd_ The end iterator pointing to one past the last character making up this token.
     * @param containingBegin_ The begin iterator pointing to the first character of the logical argument containing this token.
     * @param containingEnd_ The end iterator pointing to the last character of the logical argument containing this token.
     */
    Token(std::unique_ptr<LogicalNode> node_, const_iterator sourceBegin_, const_iterator sourceEnd_, const_iterator containingBegin_, const_iterator containingEnd_);

    /**
     * Move-constructs a token.
     * @param [in,out] other The token from which this one is move constructed.
     */
    Token(Token&& other);

    /**
     * Move assigns a token.
     * @param [in,out] other The token being move assigned.
     * @return *this.
     */
    Token& operator=(Token&& other);

    /**
     * Gets the type of this token
     * @return This token's type.
     */
    TokenType GetType() const;

    /**
     * Gets the beginning of this token.
     * @return The begin iterator pointing to the first character making up this token.
     */
    const_iterator cbegin() const;

    /**
     * Gets the end of this token.
     * @return The end iterator pointing to one past the last character making up this token.
     */
    const_iterator cend() const;

    /**
     * Gets the beginning of the logical argument containing this token.
     * @return The begin iterator pointing to the first character of the logical argument containing this token.
     */
    const_iterator argument_cbegin() const;

    /**
     * Gets the end of the logical argument containing this token.
     * @return The end iterator pointing to the last character of the logical argument containing this token.
     */
    const_iterator argument_cend() const;
private:
    const_iterator sourceBegin;
    const_iterator sourceEnd;
    const_iterator containingBegin;
    const_iterator containingEnd;
    TokenType type;
    std::unique_ptr<LogicalNode> node;
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
