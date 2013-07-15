#include "stdafx.h"
#include "../pevLib/Logic.hpp"
#include "CppUnitTest.h"

using namespace pevFind;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

template<> static std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<pevFind::LogicalNodeType>
    (pevFind::LogicalNodeType const& type)
{
    switch (type)
    {
    case LogicalNodeType::AND:
        return L"AND";
        break;
    case LogicalNodeType::OR:
        return L"OR";
        break;
    case LogicalNodeType::NOT:
        return L"NOT";
        break;
    case LogicalNodeType::LEAF:
        return L"LEAF";
        break;
    }

    assert(false);
    std::abort();
}

namespace pevFind { namespace tests
{		
    namespace 
    {
        std::unique_ptr<LogicalNode> MakeDummyLeaf();

        class DummyLeaf : LogicalLeaf
        {
            friend std::unique_ptr<LogicalNode> MakeDummyLeaf();
            DummyLeaf() {}
        public:
            virtual std::wstring GetName() const override
            {
                return L"DUMMY";
            }

            virtual std::unique_ptr<LogicalNode> Clone() const
            {
                return MakeDummyLeaf();
            }
        };

        std::unique_ptr<LogicalNode> MakeDummyLeaf()
        {
            std::unique_ptr<LogicalNode> result(new DummyLeaf);
            return result;
        }
    }

	TEST_CLASS(LogicTest)
	{
	public:
		TEST_METHOD(BasicConstructAnd)
		{
            std::vector<std::unique_ptr<LogicalNode>> empty;
			std::unique_ptr<LogicalNode> andNode = MakeLogicalAnd(std::move(empty));
            Assert::AreEqual(static_cast<std::wstring>(L"AND"), andNode->GetName());
            Assert::AreEqual(LogicalNodeType::AND, andNode->GetType());
            Assert::AreEqual(static_cast<std::size_t>(0), andNode->GetChildren().size());
		}

        TEST_METHOD(BasicConstructOr)
        {
            std::vector<std::unique_ptr<LogicalNode>> empty;
            std::unique_ptr<LogicalNode> andNode = MakeLogicalOr(std::move(empty));
            Assert::AreEqual(static_cast<std::wstring>(L"OR"), andNode->GetName());
            Assert::AreEqual(LogicalNodeType::OR, andNode->GetType());
            Assert::AreEqual(static_cast<std::size_t>(0), andNode->GetChildren().size());
        }

        TEST_METHOD(BasicConstructNot)
        {
            std::unique_ptr<LogicalNode> dummy = MakeDummyLeaf();
            std::unique_ptr<LogicalNode> andNode = MakeLogicalNot(std::move(dummy));
            Assert::AreEqual(static_cast<std::wstring>(L"NOT"), andNode->GetName());
            Assert::AreEqual(LogicalNodeType::NOT, andNode->GetType());
            Assert::AreEqual(static_cast<std::size_t>(1), andNode->GetChildren().size());
        }
	};

}} // namespace pevFind::tests
