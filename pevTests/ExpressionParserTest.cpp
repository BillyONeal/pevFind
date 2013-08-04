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
        TEST_METHOD(FileLoadLineResolverTest_Success)
        {
            FileLoadLineResolver uut;
            auto result = uut.LoadLineByName(GetTestDllFilePath(L"LoadLineResolverExample.txt"));
            Assert::IsTrue(result.Success());
            Assert::AreEqual(static_cast<std::wstring>(L"expected result\r\n"), result.GetLine());
            Assert::AreEqual(std::wstring(), result.GetError());
        }

        TEST_METHOD(FileLoadLineResolverTest_FailureInvalidPath)
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
        TEST_METHOD(PreconfiguredLoadLineResolverTest_Success)
        {
            PreconfiguredLoadLineResolver uut;
            uut.Add(L"file name", L"output");
            auto result = uut.LoadLineByName(L"file name");
            Assert::IsTrue(result.Success());
            Assert::AreEqual(std::wstring(L"output"), result.GetLine());
            Assert::AreEqual(std::wstring(), result.GetError());
        }

        TEST_METHOD(PreconfiguredLoadLineResolverTest_FailureNotConfigured)
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
        TEST_METHOD(SourceManagerTest_Basic)
        {
            SourceManager uut(L"Example");
            Assert::AreEqual(L'x', uut[1]);
        }

        TEST_METHOD(SourceManagerTest_WithReplacement)
        {
            SourceManager uut(L"Example --loadline#abc.txt# after");
            uut.InstallFile(8, 19, L"loaded tokens", L"abc.txt");
            std::wstring const expected(L"Example loaded tokens after");
            Assert::AreEqual(expected, uut.GetLogicalString());
        }

        TEST_METHOD(SourceManagerTest_SourceListing)
        {
            SourceManager sm(L"one two three four", L"input");
            std::wstring const expected(L"|1~input---------|");
            Assert::AreEqual(expected, sm.GenerateSourceListing());
        }

        TEST_METHOD(SourceManagerTest_SourceListingThreeDeep)
        {
            SourceManager sm(L"one two three four", L"input");
            sm.InstallFile(2, 1,L"second level of input", L"input 2");
            sm.InstallFile(3, 1,L"third level of input", L"input 3");
            std::wstring const expected(L"|1~input------------------------------------------------|\n"
                                        L"  |2~input 2-----------------------------|\n"
                                        L"   |3~input 3---------|");
            Assert::AreEqual(expected, sm.GenerateSourceListing());
        }

        TEST_METHOD(SourceManagerTest_SourceListingTwoTrees)
        {
            SourceManager sm(L"one secondload thirdload four", L"input");
            sm.InstallFile(4, 10, L"secondload", L"input2");
            sm.InstallFile(15, 9, L"thirdload", L"in3");
            std::wstring const expected(L"|1~input--------------------|\n"
                                        L"    |2~input2| |3~in3--|");
            Assert::AreEqual(expected, sm.GenerateSourceListing());
        }

        TEST_METHOD(SourceManagerTest_SourceListingOffEdge)
        {
            SourceManager sm(L"one two three four", L"input");
            sm.InstallFile(2, 1,L"second level of input", L"input 2");
            sm.InstallFile(3, 1,L"third level of input", L"input 3");
            std::wstring const expected(L"|1~input----------->\n"
                                        L"  |2~input 2------->\n"
                                        L"   |3~input 3------>");
            Assert::AreEqual(expected, sm.GenerateSourceListing(0, 20));
        }

        TEST_METHOD(SourceManagerTest_SourceListingOffScreen)
        {
            SourceManager sm(L"one two three four", L"input");
            sm.InstallFile(2, 1,L"second level of input", L"input 2");
            sm.InstallFile(34, 1,L"third level of input", L"input 3");
            std::wstring const expected(L"|1~input----------->\n"
                                        L"  |2~input 2------->");
            Assert::AreEqual(expected, sm.GenerateSourceListing(0, 20));
        }

        TEST_METHOD(SourceManagerTest_SourceListingNameTooLong)
        {
            SourceManager sm(L"one two three four", L"input");
            sm.InstallFile(2, 1, L"second level of input", L"input 2");
            sm.InstallFile(3, 1, L"third level of input", L"Insanely extremely ridiculusly long name goes here");
            std::wstring const expected(L"|1~input------------------------------------------------|\n"
                                        L"  |2~input 2-----------------------------|\n"
                                        L"   |3~g name goes here|\n"
                                        L"3: Insanely extremely ridiculusly long name goes here");
            Assert::AreEqual(expected, sm.GenerateSourceListing());
        }

        TEST_METHOD(SourceManagerTest_SourceListingEmptyFile)
        {
            SourceManager sm(L"input", L"input");
            sm.InstallFile(2, 1, L"", L"loaded");
            std::wstring const expected(L"|1~t|\n1: input");
            Assert::AreEqual(expected, sm.GenerateSourceListing());
        }

        TEST_METHOD(SourceManagerTest_SourceListingFileLength1)
        {
            SourceManager sm(L"input", L"input");
            sm.InstallFile(2, 1, L"x", L"loaded");
            std::wstring const expected(L"|1~t|\n  2\n1: input\n2: loaded");
            Assert::AreEqual(expected, sm.GenerateSourceListing());
        }

        TEST_METHOD(SourceManagerTest_SourceListingFileLength2)
        {
            SourceManager sm(L"input", L"input");
            sm.InstallFile(2, 1, L"xx", L"loaded");
            std::wstring const expected(L"|1~ut|\n  2^\n1: input\n2: loaded");
            Assert::AreEqual(expected, sm.GenerateSourceListing());
        }

        TEST_METHOD(SourceManagerTest_SourceListingFileLength3)
        {
            SourceManager sm(L"input", L"input");
            sm.InstallFile(2, 1, L"xxx", L"loaded");
            std::wstring const expected(L"|1~put|\n  |2|\n1: input\n2: loaded");
            Assert::AreEqual(expected, sm.GenerateSourceListing());
        }

        TEST_METHOD(SourceManagerTest_SourceListingFileLength4)
        {
            SourceManager sm(L"input and then some", L"input");
            sm.InstallFile(2, 1, L"xxx", L"loaded");
            std::wstring const expected(L"|1~input------------|\n  |2|\n2: loaded");
            Assert::AreEqual(expected, sm.GenerateSourceListing());
        }

        TEST_METHOD(SourceManagerTest_ZeroInput)
        {
            SourceManager sm(L"");
            Assert::AreEqual(std::wstring(), sm.GenerateSourceListing());
        }
    };
}}
