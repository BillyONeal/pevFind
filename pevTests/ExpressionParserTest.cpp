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

        TEST_METHOD(SourceManager_LiteralConstantAt)
        {
            SourceManager sm(L"string with { s inside");
            Assert::IsFalse(sm.ConstantAt(4, L"{"));
            Assert::IsTrue(sm.ConstantAt(12, L"{"));
            Assert::IsFalse(sm.ConstantAt(40, L"{"));
            SourceManager sm2(L"");
            Assert::IsTrue(sm2.ConstantAt(0, L""));
            Assert::IsFalse(sm2.ConstantAt(2, L""));
        }

        TEST_METHOD(SourceManager_InsensitiveConstantAt)
        {
            SourceManager sm(L"and AND aNd AnD { anD a }ND and");
            std::set<SourceLocation> trueLocs;
            trueLocs.insert(0);
            trueLocs.insert(4);
            trueLocs.insert(8);
            trueLocs.insert(12);
            trueLocs.insert(18);
            trueLocs.insert(28);

            CaseInsensitiveConstant and(L"and");
            for (SourceLocation idx = 0; idx < 100; ++idx)
            {
                bool expected = trueLocs.find(idx) != trueLocs.end();
                bool actual = sm.ConstantAt(idx, and);
                Assert::AreEqual(expected, actual);
            }
        }

        TEST_METHOD(SourceManagerTest_FindSuccess)
        {
            SourceManager sm(L" end quote \" ");
            auto const result = sm.FindNextCharacterAfter(1, L'"');
            Assert::AreEqual(11u, result);
        }

        TEST_METHOD(SourceManagerTest_FindFailure)
        {
            SourceManager sm(L" won't find me ");
            auto const result = sm.FindNextCharacterAfter(2, L'"');
            Assert::AreEqual(sm.size(), result);
        }

        TEST_METHOD(SourceManagerTest_FindEmpty)
        {
            SourceManager sm(L"");
            auto const result = sm.FindNextCharacterAfter(0, L'"');
            Assert::AreEqual(0u, result);
        }

        TEST_METHOD(SourceManagerTest_FindEmptyExceedSize)
        {
            SourceManager sm(L"");
            auto const result = sm.FindNextCharacterAfter(100, L'"');
            Assert::AreEqual(0u, result);
        }
    };

    TEST_CLASS(LexicalAnalyzerTest)
    {
        void DoAssertLexical(LexicalAnalyzer& lexer, std::wstring const& raw, std::wstring const& arg, std::wstring const& parameter, bool dashed) const
        {
            Assert::IsTrue(lexer.NextLexicalToken());
            Assert::AreEqual(raw, lexer.GetLexicalTokenRaw());
            Assert::AreEqual(arg, lexer.GetLexicalTokenArgument());
            Assert::AreEqual(parameter, lexer.GetLexicalTokenParameter());
            Assert::AreEqual(dashed, lexer.IsDashedArgument());
        }

        void DoAssertLexicalEnd(LexicalAnalyzer& lexer)
        {
            std::wstring empty;
            Assert::IsFalse(lexer.NextLexicalToken());
            Assert::AreEqual(empty, lexer.GetLexicalTokenRaw());
            Assert::AreEqual(empty, lexer.GetLexicalTokenArgument());
            Assert::AreEqual(empty, lexer.GetLexicalTokenParameter());
            Assert::IsFalse(lexer.IsDashedArgument());
        }

        std::unique_ptr<ILoadLineResolver> resolver;
    public:
        TEST_METHOD_INITIALIZE(MethodSetUp)
        {
            resolver.reset(new PreconfiguredLoadLineResolver);
        }

        TEST_METHOD(LexicalAnalyzerTest_BasicLexicalTokenWalk)
        {
            LexicalAnalyzer uut(std::move(resolver), L"  argument --arg");
            DoAssertLexical(uut, L"argument", L"argument", L"", false);
            DoAssertLexical(uut, L"--arg", L"arg", L"", true);
            DoAssertLexicalEnd(uut);
        }

        TEST_METHOD(LexicalAnalyzerTest_QuotedArgument)
        {
            LexicalAnalyzer uut(std::move(resolver), L"  \"argument spaced out\" \"--arg literal\"");
            DoAssertLexical(uut, L"\"argument spaced out\"", L"argument spaced out", L"", false);
            DoAssertLexical(uut, L"\"--arg literal\"", L"--arg literal", L"", false);
            DoAssertLexicalEnd(uut);
        }

        TEST_METHOD(LexicalAnalyzerTest_Empty)
        {
            LexicalAnalyzer uut(std::move(resolver), L"");
            DoAssertLexicalEnd(uut);
        }

        TEST_METHOD(LexicalAnalyzerTest_EmptyQuotes)
        {
            LexicalAnalyzer uut(std::move(resolver), L"\"\"");
            DoAssertLexical(uut, L"\"\"", L"", L"", false);
            DoAssertLexicalEnd(uut);
        }

        TEST_METHOD(LexicalAnalyzerTest_JustDashes)
        {
            LexicalAnalyzer uut(std::move(resolver), L"  \"----\" -- ----");
            DoAssertLexical(uut, L"\"----\"", L"----", L"", false);
            DoAssertLexical(uut, L"--", L"", L"", true);
            DoAssertLexical(uut, L"----", L"", L"", true);
            DoAssertLexicalEnd(uut);
        }

        TEST_METHOD(LexicalAnalyzerTest_Full)
        {
            LexicalAnalyzer uut(std::move(resolver), L"argument");
            DoAssertLexical(uut, L"argument", L"argument", L"", false);
            DoAssertLexicalEnd(uut);
        }

        TEST_METHOD(LexicalAnalyzerTest_FullDash)
        {
            LexicalAnalyzer uut(std::move(resolver), L"--argument");
            DoAssertLexical(uut, L"--argument", L"argument", L"", true);
            DoAssertLexicalEnd(uut);
        }

        TEST_METHOD(LexicalAnalyzerTest_SimpleQuotedParameter)
        {
            LexicalAnalyzer uut(std::move(resolver), L"--argument\"param\" baz");
            DoAssertLexical(uut, L"--argument\"param\"", L"argument", L"param", true);
            DoAssertLexical(uut, L"baz", L"baz", L"", false);
            DoAssertLexicalEnd(uut);
        }

        TEST_METHOD(LexicalAnalyzerTest_SimpleQuotedParameterEnd)
        {
            LexicalAnalyzer uut(std::move(resolver), L"--argument\"param\"");
            DoAssertLexical(uut, L"--argument\"param\"", L"argument", L"param", true);
            DoAssertLexicalEnd(uut);
        }
    };

    TEST_CLASS(CaseInsensitiveConstantTest)
    {
    public:
        TEST_METHOD(CaseInsensitiveConstantTest_Empty)
        {
            CaseInsensitiveConstant empty;
            Assert::AreEqual(0u, empty.size());
            Assert::AreEqual<std::wstring>(L"", empty.lower_cstr());
            Assert::AreEqual<std::wstring>(L"", empty.upper_cstr());
            Assert::AreEqual<void const*>(empty.lcbegin(), empty.lcend());
            Assert::AreEqual<void const*>(empty.ucbegin(), empty.ucend());
        }

        TEST_METHOD(CaseInsensitiveConstantTest_Basic)
        {
            CaseInsensitiveConstant basic(L"Case String");
            Assert::AreEqual(11u, basic.size());
            Assert::AreEqual<std::wstring>(L"case string", basic.lower_cstr());
            Assert::AreEqual<std::wstring>(L"CASE STRING", basic.upper_cstr());
            Assert::AreEqual<std::wstring>(L"case string", std::wstring(basic.lcbegin(), basic.lcend()));
            Assert::AreEqual<std::wstring>(L"CASE STRING", std::wstring(basic.ucbegin(), basic.ucend()));
        }

        TEST_METHOD(CaseInsensitiveConstantTest_AssignmentOperator)
        {
            CaseInsensitiveConstant hello(L"Hello !"), world(L"World");
            hello = world;
            Assert::AreEqual(5u, hello.size());
            Assert::AreEqual<std::wstring>(L"world", hello.lower_cstr());
            Assert::AreEqual<std::wstring>(L"WORLD", hello.upper_cstr());
            Assert::AreEqual<std::wstring>(L"world", std::wstring(hello.lcbegin(), hello.lcend()));
            Assert::AreEqual<std::wstring>(L"WORLD", std::wstring(hello.ucbegin(), hello.ucend()));
            Assert::AreEqual(5u, world.size());
            Assert::AreEqual<std::wstring>(L"world", world.lower_cstr());
            Assert::AreEqual<std::wstring>(L"WORLD", world.upper_cstr());
            Assert::AreEqual<std::wstring>(L"world", std::wstring(world.lcbegin(), world.lcend()));
            Assert::AreEqual<std::wstring>(L"WORLD", std::wstring(world.ucbegin(), world.ucend()));
            hello = std::move(world);
            Assert::AreEqual(5u, hello.size());
            Assert::AreEqual<std::wstring>(L"world", hello.lower_cstr());
            Assert::AreEqual<std::wstring>(L"WORLD", hello.upper_cstr());
            Assert::AreEqual<std::wstring>(L"world", std::wstring(hello.lcbegin(), hello.lcend()));
            Assert::AreEqual<std::wstring>(L"WORLD", std::wstring(hello.ucbegin(), hello.ucend()));
            Assert::AreEqual(0u, world.size());
            Assert::AreEqual<std::wstring>(L"", world.lower_cstr());
            Assert::AreEqual<std::wstring>(L"", world.upper_cstr());
            Assert::AreEqual<std::wstring>(L"", std::wstring(world.lcbegin(), world.lcend()));
            Assert::AreEqual<std::wstring>(L"", std::wstring(world.ucbegin(), world.ucend()));
            world = L"tEsT";
            Assert::AreEqual(4u, world.size());
            Assert::AreEqual<std::wstring>(L"test", world.lower_cstr());
            Assert::AreEqual<std::wstring>(L"TEST", world.upper_cstr());
            Assert::AreEqual<std::wstring>(L"test", std::wstring(world.lcbegin(), world.lcend()));
            Assert::AreEqual<std::wstring>(L"TEST", std::wstring(world.ucbegin(), world.ucend()));
            world = world;
            Assert::AreEqual(4u, world.size());
            Assert::AreEqual<std::wstring>(L"test", world.lower_cstr());
            Assert::AreEqual<std::wstring>(L"TEST", world.upper_cstr());
            Assert::AreEqual<std::wstring>(L"test", std::wstring(world.lcbegin(), world.lcend()));
            Assert::AreEqual<std::wstring>(L"TEST", std::wstring(world.ucbegin(), world.ucend()));
            world = std::move(world);
            Assert::AreEqual(4u, world.size());
            Assert::AreEqual<std::wstring>(L"test", world.lower_cstr());
            Assert::AreEqual<std::wstring>(L"TEST", world.upper_cstr());
            Assert::AreEqual<std::wstring>(L"test", std::wstring(world.lcbegin(), world.lcend()));
            Assert::AreEqual<std::wstring>(L"TEST", std::wstring(world.ucbegin(), world.ucend()));
        }


        TEST_METHOD(CaseInsensitiveConstantTest_AssignmentMemberFunction)
        {
            CaseInsensitiveConstant hello(L"Hello !"), world(L"World");
            hello.assign(world);
            Assert::AreEqual(5u, hello.size());
            Assert::AreEqual<std::wstring>(L"world", hello.lower_cstr());
            Assert::AreEqual<std::wstring>(L"WORLD", hello.upper_cstr());
            Assert::AreEqual<std::wstring>(L"world", std::wstring(hello.lcbegin(), hello.lcend()));
            Assert::AreEqual<std::wstring>(L"WORLD", std::wstring(hello.ucbegin(), hello.ucend()));
            Assert::AreEqual(5u, world.size());
            Assert::AreEqual<std::wstring>(L"world", world.lower_cstr());
            Assert::AreEqual<std::wstring>(L"WORLD", world.upper_cstr());
            Assert::AreEqual<std::wstring>(L"world", std::wstring(world.lcbegin(), world.lcend()));
            Assert::AreEqual<std::wstring>(L"WORLD", std::wstring(world.ucbegin(), world.ucend()));
            hello.assign(std::move(world));
            Assert::AreEqual(5u, hello.size());
            Assert::AreEqual<std::wstring>(L"world", hello.lower_cstr());
            Assert::AreEqual<std::wstring>(L"WORLD", hello.upper_cstr());
            Assert::AreEqual<std::wstring>(L"world", std::wstring(hello.lcbegin(), hello.lcend()));
            Assert::AreEqual<std::wstring>(L"WORLD", std::wstring(hello.ucbegin(), hello.ucend()));
            Assert::AreEqual(0u, world.size());
            Assert::AreEqual<std::wstring>(L"", world.lower_cstr());
            Assert::AreEqual<std::wstring>(L"", world.upper_cstr());
            Assert::AreEqual<std::wstring>(L"", std::wstring(world.lcbegin(), world.lcend()));
            Assert::AreEqual<std::wstring>(L"", std::wstring(world.ucbegin(), world.ucend()));
            world.assign(L"tEsT");
            Assert::AreEqual(4u, world.size());
            Assert::AreEqual<std::wstring>(L"test", world.lower_cstr());
            Assert::AreEqual<std::wstring>(L"TEST", world.upper_cstr());
            Assert::AreEqual<std::wstring>(L"test", std::wstring(world.lcbegin(), world.lcend()));
            Assert::AreEqual<std::wstring>(L"TEST", std::wstring(world.ucbegin(), world.ucend()));
            hello.assign(L"he\0lo", 4);
            Assert::AreEqual(4u, hello.size());
            std::wstring lower(L"he\0l", 4);
            std::wstring upper(L"HE\0L", 4);
            Assert::AreEqual<std::wstring>(L"he", hello.lower_cstr());
            Assert::AreEqual<std::wstring>(L"HE", hello.upper_cstr());
            Assert::AreEqual<std::wstring>(lower, std::wstring(hello.lcbegin(), hello.lcend()));
            Assert::AreEqual<std::wstring>(upper, std::wstring(hello.ucbegin(), hello.ucend()));
            hello.assign(hello);
            Assert::AreEqual(4u, hello.size());
            Assert::AreEqual<std::wstring>(L"he", hello.lower_cstr());
            Assert::AreEqual<std::wstring>(L"HE", hello.upper_cstr());
            Assert::AreEqual<std::wstring>(lower, std::wstring(hello.lcbegin(), hello.lcend()));
            Assert::AreEqual<std::wstring>(upper, std::wstring(hello.ucbegin(), hello.ucend()));
            hello.assign(std::move(hello));
            Assert::AreEqual(4u, hello.size());
            Assert::AreEqual<std::wstring>(L"he", hello.lower_cstr());
            Assert::AreEqual<std::wstring>(L"HE", hello.upper_cstr());
            Assert::AreEqual<std::wstring>(lower, std::wstring(hello.lcbegin(), hello.lcend()));
            Assert::AreEqual<std::wstring>(upper, std::wstring(hello.ucbegin(), hello.ucend()));
        }

        TEST_METHOD(CaseInsensitiveConstantTest_LengthString)
        {
            CaseInsensitiveConstant hello(L"he\0lo", 4);
            Assert::AreEqual(4u, hello.size());
            std::wstring lower(L"he\0l", 4);
            std::wstring upper(L"HE\0L", 4);
            Assert::AreEqual<std::wstring>(L"he", hello.lower_cstr());
            Assert::AreEqual<std::wstring>(L"HE", hello.upper_cstr());
            Assert::AreEqual<std::wstring>(lower, std::wstring(hello.lcbegin(), hello.lcend()));
            Assert::AreEqual<std::wstring>(upper, std::wstring(hello.ucbegin(), hello.ucend()));
        }

        TEST_METHOD(CaseInsensitiveConstantTest_Swap)
        {
            using std::swap;
            CaseInsensitiveConstant hello(L"Hello"), world(L"World");
            hello.swap(world);
            Assert::AreEqual(L"world", hello.lower_cstr());
            Assert::AreEqual(L"hello", world.lower_cstr());

            swap(world, hello);
            Assert::AreEqual(L"hello", hello.lower_cstr());
            Assert::AreEqual(L"world", world.lower_cstr());
            CaseInsensitiveConstant().swap(hello);
            Assert::AreEqual(0u, hello.size());
        }

        TEST_METHOD(CaseInsensitiveConstantTest_StdString)
        {
            std::wstring example(L"Example");
            CaseInsensitiveConstant uut(example);
            Assert::AreEqual(7u, uut.size());
            Assert::AreEqual<std::wstring>(L"example", uut.lower_cstr());
            Assert::AreEqual<std::wstring>(L"EXAMPLE", uut.upper_cstr());
            Assert::AreEqual<std::wstring>(L"example", std::wstring(uut.lcbegin(), uut.lcend()));
            Assert::AreEqual<std::wstring>(L"EXAMPLE", std::wstring(uut.ucbegin(), uut.ucend()));

            CaseInsensitiveConstant uut2;
            uut2 = example;
            Assert::AreEqual(7u, uut2.size());
            Assert::AreEqual<std::wstring>(L"example", uut2.lower_cstr());
            Assert::AreEqual<std::wstring>(L"EXAMPLE", uut2.upper_cstr());
            Assert::AreEqual<std::wstring>(L"example", std::wstring(uut2.lcbegin(), uut2.lcend()));
            Assert::AreEqual<std::wstring>(L"EXAMPLE", std::wstring(uut2.ucbegin(), uut2.ucend()));

            CaseInsensitiveConstant uut3;
            uut3.assign(example);
            Assert::AreEqual(7u, uut3.size());
            Assert::AreEqual<std::wstring>(L"example", uut3.lower_cstr());
            Assert::AreEqual<std::wstring>(L"EXAMPLE", uut3.upper_cstr());
            Assert::AreEqual<std::wstring>(L"example", std::wstring(uut3.lcbegin(), uut3.lcend()));
            Assert::AreEqual<std::wstring>(L"EXAMPLE", std::wstring(uut3.ucbegin(), uut3.ucend()));
        }
    };

}}
