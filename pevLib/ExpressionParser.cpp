//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "pch.hpp"
#include "ExpressionParser.hpp"
#include "../LogCommon/Win32Exception.hpp"
#include "utility.h"

static std::wstring emptyString;

namespace pevFind
{
    Token::Token(TokenType type_, std::wstring::const_iterator sourceBegin_, std::wstring::const_iterator sourceEnd_, std::wstring::const_iterator containingBegin_, std::wstring::const_iterator containingEnd_)
        : type(type_)
        , sourceBegin(sourceBegin_)
        , sourceEnd(sourceEnd_)
        , containingBegin(containingBegin_)
        , containingEnd(containingEnd_)
        , node(nullptr)
    {
    }

    Token::Token(std::unique_ptr<LogicalNode> node_, std::wstring::const_iterator sourceBegin_, std::wstring::const_iterator sourceEnd_, std::wstring::const_iterator containingBegin_, std::wstring::const_iterator containingEnd_)
        : type(TokenType::Condition)
        , sourceBegin(sourceBegin_)
        , sourceEnd(sourceEnd_)
        , containingBegin(containingBegin_)
        , containingEnd(containingEnd_)
        , node(std::move(node_))
    {
    }

    Token::Token(Token&& other)
        : type(other.type)
        , sourceBegin(other.sourceBegin)
        , sourceEnd(other.sourceEnd)
        , containingBegin(other.containingBegin)
        , containingEnd(other.containingEnd)
        , node(std::move(other.node))
    {
    }

    Token& Token::operator=(Token&& other)
    {
        this->type = other.type;
        this->sourceBegin = other.sourceEnd;
        this->sourceEnd = other.sourceEnd;
        this->containingBegin = other.containingBegin;
        this->containingEnd = other.containingEnd;
        this->node = std::move(other.node);

        return *this;
    }

    TokenType Token::GetType() const
    {
        return this->type;
    }

    std::wstring::const_iterator Token::cbegin() const
    {
        return this->sourceBegin;
    }

    std::wstring::const_iterator Token::cend() const
    {
        return this->sourceEnd;
    }

    std::wstring::const_iterator Token::argument_cbegin() const
    {
        return this->containingBegin;
    }

    std::wstring::const_iterator Token::argument_cend() const
    {
        return this->containingEnd;
    }

    LoadLineResult::LoadLineResult(std::wstring&& lineOrError_, bool success_)
        : lineOrError(std::move(lineOrError_))
        , success(success_)
    {
    }

    LoadLineResult LoadLineResult::FromLineValue(std::wstring line)
    {
        return LoadLineResult(std::move(line), true);
    }

    LoadLineResult LoadLineResult::FromFailure(std::wstring error)
    {
        return LoadLineResult(std::move(error), false);
    }

    std::wstring const& LoadLineResult::GetLine() const
    {
        if (success)
        {
            return lineOrError;
        }
        else
        {
            return emptyString;
        }
    }

    bool LoadLineResult::Success() const
    {
        return success;
    }

    std::wstring const& LoadLineResult::GetError() const
    {
        if (success)
        {
            return emptyString;
        }
        else
        {
            return lineOrError;
        }
    }

    ILoadLineResolver::~ILoadLineResolver()
    {
    }

    LoadLineResult FileLoadLineResolver::LoadLineByName(std::wstring const& name) const
    {
        try
        {
            return LoadLineResult::FromLineValue(loadFileAsString(name));
        }
        catch (std::bad_alloc const&)
        {
            return LoadLineResult::FromFailure(L"The file \"" + name + L"\" was too large to load as a command line source.");
        }
        catch (Instalog::SystemFacades::Win32Exception const& error)
        {
            return LoadLineResult::FromFailure(L"Failed opening \"" + name + L"\" with error " + error.GetWideMessage());
        }
    }

    void PreconfiguredLoadLineResolver::Add(std::wstring&& name, std::wstring&& value)
    {
        lines.insert(std::make_pair(std::move(name), std::move(value)));
    }

    LoadLineResult PreconfiguredLoadLineResolver::LoadLineByName(std::wstring const& name) const
    {
        auto const namePosition = lines.find(name);
        if (namePosition == lines.cend())
        {
            return LoadLineResult::FromFailure(L"Line was not configured.");
        }
        else
        {
            return LoadLineResult::FromLineValue(namePosition->second);
        }
    }
}
