#include "pch.hpp"
#include "Logic.hpp"
#include <cassert>
#include <stack>
#include <iterator>
#include <cstdint>

namespace
{
    using namespace pevFind;

    struct NotCaller
    {
        std::unique_ptr<LogicalNode> operator()(std::unique_ptr<LogicalNode>&& input)
        {
            return MakeLogicalNot(std::move(input));
        }
    };

    std::unique_ptr<LogicalNode> MakeNegationNormal(std::unique_ptr<LogicalNode> source, std::size_t notCrosses);

    class MakeNegationNormalCaller
    {
        std::size_t notCrossCount;
    public:
        explicit MakeNegationNormalCaller(std::size_t notCrossCount_)
            : notCrossCount(notCrossCount_)
        {
        }
        std::unique_ptr<LogicalNode> operator()(std::unique_ptr<LogicalNode>&& node)
        {
            return MakeNegationNormal(std::move(node), notCrossCount);
        }
    };

    std::unique_ptr<LogicalNode> MakeNegationNormal(std::unique_ptr<LogicalNode> source, std::size_t notCrosses)
    {
        LogicalNodeType currentType = source->GetType();
        bool noFlip = (notCrosses % 2) == 0;
        if (currentType == LogicalNodeType::LEAF)
        {
            if (noFlip)
            {
                return std::move(source);
            }
            else
            {
                return MakeLogicalNot(std::move(source));
            }
        }
        else if (currentType == LogicalNodeType::AND || currentType == LogicalNodeType::OR)
        {
            std::vector<std::unique_ptr<LogicalNode>> children(StealChildren(std::move(source)));
            std::transform(
                std::make_move_iterator(children.begin()),
                std::make_move_iterator(children.end()),
                children.begin(),
                static_cast<MakeNegationNormalCaller>(notCrosses));

            bool isAnd = currentType == LogicalNodeType::AND;
            bool isOr = currentType == LogicalNodeType::OR;
            if ((noFlip && isAnd) || (!noFlip && isOr))
            {
                return MakeLogicalAnd(std::move(children));
            }
            else
            {
                return MakeLogicalOr(std::move(children));
            }
        }
        else
        {
            assert(currentType == LogicalNodeType::NOT);
            LogicalNodeType childType = source->GetChildren()[0]->GetType();
            if (childType == LogicalNodeType::LEAF)
            {
                if (noFlip)
                {
                    return std::move(source);
                }
                else
                {
                    return StealFirstChild(std::move(source));
                }
            }
            else
            {
                return MakeNegationNormal(StealFirstChild(std::move(source)), notCrosses + 1);
            }
        }
    }
}

namespace pevFind
{
    const std::vector<std::unique_ptr<LogicalNode>> LogicalLeaf::childrenVector;

    std::vector<std::unique_ptr<LogicalNode>> CloneChildren(LogicalNode const* source)
    {
        auto const& children = source->GetChildren();
        std::vector<std::unique_ptr<LogicalNode>> result;
        result.reserve(children.size());
        for (auto const& child : children)
        {
            result.emplace_back(child->Clone());
        }

        return result;
    }

    LogicalCombination::LogicalCombination(std::vector<std::unique_ptr<LogicalNode>> children_, LogicalNodeType type_)
        : children(std::move(children_))
        , type(type_)
    {
        // Make sure that this is a combination type.
        assert(type != LogicalNodeType::LEAF);
        // Make sure that if this is a NOT, that there is only one child.
        assert(type != LogicalNodeType::NOT || children.size() == 1);
    }

    std::vector<std::unique_ptr<LogicalNode>> const& LogicalCombination::GetChildren() const throw()
    {
        return children;
    }

    std::wstring LogicalCombination::GetName() const
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
        default:
            assert(false);
            std::abort();
        }
    }

    std::unique_ptr<LogicalNode> LogicalCombination::Clone() const
    {
        std::unique_ptr<LogicalNode> result(new LogicalCombination(CloneChildren(this), type));
        return result;
    }

    LogicalNodeType LogicalCombination::GetType() const throw()
    {
        return type;
    }

    std::unique_ptr<LogicalNode> MakeLogicalAnd(std::vector<std::unique_ptr<LogicalNode>> children)
    {
        std::unique_ptr<LogicalNode> result(new LogicalCombination(std::move(children), LogicalNodeType::AND));
        return result;
    }

    std::unique_ptr<LogicalNode> MakeLogicalOr(std::vector<std::unique_ptr<LogicalNode>> children)
    {
        std::unique_ptr<LogicalNode> result(new LogicalCombination(std::move(children), LogicalNodeType::OR));
        return result;
    }

    std::unique_ptr<LogicalNode> MakeLogicalNot(std::unique_ptr<LogicalNode> child)
    {
        if (child->GetType() == LogicalNodeType::NOT)
        {
            // If the child is a not, avoid double negatives by returning the child directly.
            // Child (itself a not) is destroyed when this function returns.
            return StealFirstChild(std::move(child));
        }
        else
        {
            // If the child is not a not, then create a new LogicalNot containing child.
            std::vector<std::unique_ptr<LogicalNode>> children;
            children.emplace_back(std::move(child));
            std::unique_ptr<LogicalNode> result(new LogicalCombination(std::move(children), LogicalNodeType::NOT));
            return result;
        }
    }

    LogicalNodeType LogicalLeaf::GetType() const throw()
    {
        return LogicalNodeType::LEAF;
    }

    std::vector<std::unique_ptr<LogicalNode>> const& LogicalLeaf::GetChildren() const throw()
    {
        return childrenVector;
    }

    std::unique_ptr<LogicalNode> StealFirstChild(std::unique_ptr<LogicalNode> target) throw()
    {
        assert(target->GetType() != LogicalNodeType::LEAF);
        assert(target->GetChildren().size() >= 1u);
        return std::move(static_cast<LogicalCombination*>(target.get())->children[0]);
    }

    std::vector<std::unique_ptr<LogicalNode>> StealChildren(std::unique_ptr<LogicalNode> target) throw()
    {
        assert(target->GetType() != LogicalNodeType::LEAF);
        return std::move(static_cast<LogicalCombination*>(target.get())->children);
    }

    std::unique_ptr<LogicalNode> ApplyDemorgan(std::unique_ptr<LogicalNode> source)
    {
        assert(source->GetType() != LogicalNodeType::LEAF);
        LogicalNodeType sourceType = source->GetType();
        bool sourceNot = false;
        if (sourceType == LogicalNodeType::NOT)
        {
            sourceNot = true;
            source = StealFirstChild(std::move(source));
            sourceType = source->GetType();
        }

        assert(sourceType == LogicalNodeType::AND || sourceType == LogicalNodeType::OR);
        std::vector<std::unique_ptr<LogicalNode>> children(StealChildren(std::move(source)));
        // Negate the children.
        std::transform(
            std::make_move_iterator(children.begin()),
            std::make_move_iterator(children.end()),
            children.begin(),
            NotCaller());
        
        std::unique_ptr<LogicalNode> result;
        if (sourceType == LogicalNodeType::AND)
        {
            result = MakeLogicalOr(std::move(children));
        }
        else
        {
            result = MakeLogicalAnd(std::move(children));
        }

        if (sourceNot)
        {
            return result;
        }
        else
        {
            return MakeLogicalNot(std::move(result));
        }
    }

    std::unique_ptr<LogicalNode> MakeNegationNormal(std::unique_ptr<LogicalNode> source)
    {
        return ::MakeNegationNormal(std::move(source), 0);
    }

    bool IsLiteral(LogicalNode const* node) throw()
    {
        auto currentType = node->GetType();
        return currentType == LogicalNodeType::LEAF
            || ((currentType == LogicalNodeType::NOT) && node->GetChildren()[0]->GetType() == LogicalNodeType::LEAF);
    }

    std::wstring GetLiteralName(LogicalNode const* node)
    {
        assert(IsLiteral(node));
        auto currentType = node->GetType();
        if (currentType == LogicalNodeType::LEAF)
        {
            return L"- " + node->GetName();
        }
        else
        {
            return L"- NOT( " + node->GetChildren()[0]->GetName() + L" )";
        }
    }

    std::wstring MakeString(LogicalNode const* source)
    {
        std::wstring result;
        struct Frame
        {
            LogicalNode const* node;
            std::size_t currentChild;
        };
        std::stack<Frame, std::vector<Frame>> stack;
        Frame first = {source, 0};
        stack.push(first);
        while (!stack.empty())
        {
            Frame current = stack.top();
            stack.pop();
            if (current.currentChild == 0)
            {
                result.append(stack.size(), L' ');
                if (IsLiteral(current.node))
                {
                    result.append(GetLiteralName(current.node));
                    result.push_back(L'\n');
                    continue;
                }
                else
                {
                    result.append(L"+ ");
                    result.append(current.node->GetName());
                    result.push_back(L'\n');
                }
            }

            if (current.node->GetChildren().size() > current.currentChild)
            {
                auto oldChild = current.currentChild++;
                stack.push(current);
                Frame next = { current.node->GetChildren()[oldChild].get(), 0 };
                stack.push(next);
            }
        }

        return result;
    }
}

