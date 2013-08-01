//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdafx.h"
#include "../pevLib/ExpressionParser.hpp"
#include "CppUnitTest.h"
#include "TestSupport.hpp"

using namespace pevFind;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace pevFind { namespace tests
{

    TEST_CLASS(FileLoadLineResolverTest)
    {
    public:
        TEST_METHOD(LoadLineFileResolverSuccess)
        {
            FileLoadLineResolver uut;
            auto result = uut.LoadLineByName(GetTestDllFilePath(L"LoadLineResolverExample.txt"));
            Assert::IsTrue(result.Success());
            Assert::AreEqual(static_cast<std::wstring>(L"expected result\r\n"), result.GetLine());
            Assert::AreEqual(std::wstring(), result.GetError());
        }

        TEST_METHOD(LoadLineFileResolverFailureInvalidPath)
        {
            FileLoadLineResolver uut;
            auto result = uut.LoadLineByName(L"Invalid path here :: ");
            Assert::IsFalse(result.Success());
            Assert::AreEqual(std::wstring(), result.GetLine());
            Assert::AreEqual(static_cast<std::wstring>(L"Failed opening \"Invalid path here :: \" with error The filename, directory name, or volume label syntax is incorrect.\r\n"),
                result.GetError());
        }
    };

    TEST_CLASS(PreconfiguredLoadLineResolverTest)
    {
    public:
        TEST_METHOD(PreconfiguredLoadLineResolverSuccess)
        {
            PreconfiguredLoadLineResolver uut;
            uut.Add(L"file name", L"output");
            auto result = uut.LoadLineByName(L"file name");
            Assert::IsTrue(result.Success());
            Assert::AreEqual(std::wstring(L"output"), result.GetLine());
            Assert::AreEqual(std::wstring(), result.GetError());
        }

        TEST_METHOD(PreconfiguredLoadLineResolverFailureNotConfigured)
        {
            PreconfiguredLoadLineResolver uut;
            uut.Add(L"file name", L"output");
            auto result = uut.LoadLineByName(L"file name nonexistent");
            Assert::IsFalse(result.Success());
            Assert::AreEqual(std::wstring(), result.GetLine());
            Assert::AreEqual(std::wstring(L"Line was not configured."), result.GetError());
        }
    };

    TEST_CLASS(SourceManagerTest)
    {
    public:
        TEST_METHOD(Basic)
        {
            SourceManager uut(L"Example");
            Assert::AreEqual(L'x', uut[1]);
        }

        TEST_METHOD(WithReplacement)
        {
            SourceManager uut(L"Example --loadline#abc.txt# after");
            uut.InstallFile(8, 19, L"loaded tokens", L"abc.txt");
            std::wstring const expected(L"Example loaded tokens after");
            Assert::AreEqual(expected, uut.GetLogicalString());
        }
    };
}}
