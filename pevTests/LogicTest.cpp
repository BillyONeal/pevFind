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

        std::unique_ptr<LogicalNode> MakeNamedLeaf(std::wstring name);

        class NamedLeaf : LogicalLeaf
        {
            friend std::unique_ptr<LogicalNode> MakeNamedLeaf(std::wstring name);
            std::wstring name_;
            NamedLeaf(std::wstring name) : name_(name) {}
        public:
            virtual std::wstring GetName() const override
            {
                return name_;
            }

            virtual std::unique_ptr<LogicalNode> Clone() const
            {
                return MakeNamedLeaf(name_);
            }
        };

        std::unique_ptr<LogicalNode> MakeNamedLeaf(std::wstring name)
        {
            std::unique_ptr<LogicalNode> result(new NamedLeaf(std::move(name)));
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
            std::unique_ptr<LogicalNode> orNode = MakeLogicalOr(std::move(empty));
            Assert::AreEqual(static_cast<std::wstring>(L"OR"), orNode->GetName());
            Assert::AreEqual(LogicalNodeType::OR, orNode->GetType());
            Assert::AreEqual(static_cast<std::size_t>(0), orNode->GetChildren().size());
        }

        TEST_METHOD(BasicConstructNot)
        {
            std::unique_ptr<LogicalNode> dummy = MakeDummyLeaf();
            std::unique_ptr<LogicalNode> notNode = MakeLogicalNot(std::move(dummy));
            Assert::AreEqual(static_cast<std::wstring>(L"NOT"), notNode->GetName());
            Assert::AreEqual(LogicalNodeType::NOT, notNode->GetType());
            Assert::AreEqual(static_cast<std::size_t>(1), notNode->GetChildren().size());
        }

        TEST_METHOD(BasicConstructNotNot)
        {
            std::unique_ptr<LogicalNode> dummy = MakeDummyLeaf();
            std::unique_ptr<LogicalNode> notNode = MakeLogicalNot(std::move(dummy));
            std::unique_ptr<LogicalNode> notNotNode = MakeLogicalNot(std::move(notNode));
            Assert::AreEqual(static_cast<std::wstring>(L"DUMMY"), notNotNode->GetName());
            Assert::AreEqual(LogicalNodeType::LEAF, notNotNode->GetType());
            Assert::AreEqual(static_cast<std::size_t>(0), notNotNode->GetChildren().size());
        }

        TEST_METHOD(BasicDemorganAnd)
        {
            std::vector<std::unique_ptr<LogicalNode>> andChildren;
            andChildren.push_back(MakeNamedLeaf(L"A"));
            andChildren.push_back(MakeLogicalNot(MakeNamedLeaf(L"B")));
            auto andNode = MakeLogicalAnd(std::move(andChildren));
            auto result = ApplyDemorgan(std::move(andNode));
            Assert::AreEqual(LogicalNodeType::NOT, result->GetType());
            auto expectedOr = StealFirstChild(std::move(result));
            Assert::AreEqual(LogicalNodeType::OR, expectedOr->GetType());
            std::vector<std::unique_ptr<LogicalNode>> orChildren(StealChildren(std::move(expectedOr)));
            Assert::AreEqual(static_cast<std::size_t>(2), orChildren.size());
            Assert::AreEqual(LogicalNodeType::NOT, orChildren[0]->GetType());
            Assert::AreEqual(static_cast<std::wstring>(L"A"), orChildren[0]->GetChildren()[0]->GetName());
            Assert::AreEqual(LogicalNodeType::LEAF, orChildren[1]->GetType());
            Assert::AreEqual(static_cast<std::wstring>(L"B"), orChildren[1]->GetName());
        }

        TEST_METHOD(BasicDemorganOr)
        {
            std::vector<std::unique_ptr<LogicalNode>> orChildren;
            orChildren.push_back(MakeNamedLeaf(L"A"));
            orChildren.push_back(MakeLogicalNot(MakeNamedLeaf(L"B")));
            auto orNode = MakeLogicalOr(std::move(orChildren));
            auto result = ApplyDemorgan(std::move(orNode));
            Assert::AreEqual(LogicalNodeType::NOT, result->GetType());
            auto expectedAnd = StealFirstChild(std::move(result));
            Assert::AreEqual(LogicalNodeType::AND, expectedAnd->GetType());
            std::vector<std::unique_ptr<LogicalNode>> andChildren(StealChildren(std::move(expectedAnd)));
            Assert::AreEqual(static_cast<std::size_t>(2), andChildren.size());
            Assert::AreEqual(LogicalNodeType::NOT, andChildren[0]->GetType());
            Assert::AreEqual(static_cast<std::wstring>(L"A"), andChildren[0]->GetChildren()[0]->GetName());
            Assert::AreEqual(LogicalNodeType::LEAF, andChildren[1]->GetType());
            Assert::AreEqual(static_cast<std::wstring>(L"B"), andChildren[1]->GetName());
        }

        TEST_METHOD(BasicDemorganNotAnd)
        {
            std::vector<std::unique_ptr<LogicalNode>> andChildren;
            andChildren.push_back(MakeNamedLeaf(L"A"));
            andChildren.push_back(MakeLogicalNot(MakeNamedLeaf(L"B")));
            auto andNode = MakeLogicalAnd(std::move(andChildren));
            auto expectedOr = ApplyDemorgan(MakeLogicalNot(std::move(andNode)));
            Assert::AreEqual(LogicalNodeType::OR, expectedOr->GetType());
            std::vector<std::unique_ptr<LogicalNode>> orChildren(StealChildren(std::move(expectedOr)));
            Assert::AreEqual(static_cast<std::size_t>(2), orChildren.size());
            Assert::AreEqual(LogicalNodeType::NOT, orChildren[0]->GetType());
            Assert::AreEqual(static_cast<std::wstring>(L"A"), orChildren[0]->GetChildren()[0]->GetName());
            Assert::AreEqual(LogicalNodeType::LEAF, orChildren[1]->GetType());
            Assert::AreEqual(static_cast<std::wstring>(L"B"), orChildren[1]->GetName());
        }

        TEST_METHOD(BasicDemorganNotOr)
        {
            std::vector<std::unique_ptr<LogicalNode>> orChildren;
            orChildren.push_back(MakeNamedLeaf(L"A"));
            orChildren.push_back(MakeLogicalNot(MakeNamedLeaf(L"B")));
            auto orNode = MakeLogicalOr(std::move(orChildren));
            auto expectedAnd = ApplyDemorgan(MakeLogicalNot(std::move(orNode)));
            Assert::AreEqual(LogicalNodeType::AND, expectedAnd->GetType());
            std::vector<std::unique_ptr<LogicalNode>> andChildren(StealChildren(std::move(expectedAnd)));
            Assert::AreEqual(static_cast<std::size_t>(2), andChildren.size());
            Assert::AreEqual(LogicalNodeType::NOT, andChildren[0]->GetType());
            Assert::AreEqual(static_cast<std::wstring>(L"A"), andChildren[0]->GetChildren()[0]->GetName());
            Assert::AreEqual(LogicalNodeType::LEAF, andChildren[1]->GetType());
            Assert::AreEqual(static_cast<std::wstring>(L"B"), andChildren[1]->GetName());
        }
	};

}} // namespace pevFind::tests
