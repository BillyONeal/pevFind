//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once
#include <memory>
#include <cstdint>
#include <vector>
#include <stack>
#include <iterator>
#include <string>
#include <boost/noncopyable.hpp>

namespace pevFind
{
    /**
     * Values that represent the type of a logical node.
     */
    enum class LogicalNodeType
    {
        ///< An enum constant representing the and option
        AND,
        ///< An enum constant representing the or option
        OR,
        ///< An enum constant representing the not option
        NOT,
        ///< An enum constant representing the leaf option
        LEAF
    };

    /**
     * Logical node.
     */
    class LogicalNode : boost::noncopyable
    {
    public:
        typedef std::vector<std::unique_ptr<LogicalNode>> NodeVector;
        /**
         * Gets the name of this node type.
         * @return The name of this node.
         */
        virtual std::wstring GetName() const = 0;

        /**
         * Makes a deep copy of this instance.
         * @return A copy of this instance.
         */
        virtual std::unique_ptr<LogicalNode> Clone() const = 0;

        virtual NodeVector const& GetChildren() const = 0;

        NodeVector CloneChildren() const
        {
            auto const& children = this->GetChildren();
            NodeVector result(children.size());
            for (auto const& child : children)
            {
                result.emplace_back(child->Clone());
            }

            return result;
        }

        /**
         * Gets the logical type of this node.
         * @return The logical type of this node.
         */
        virtual LogicalNodeType GetType() const = 0;

        /**
         * Destroys an instance of the LogicalNode class.
         */
        virtual ~LogicalNode() {}
    };

    std::unique_ptr<LogicalNode> MakeLogicalAnd(std::vector<std::unique_ptr<LogicalNode>> children);
    std::unique_ptr<LogicalNode> MakeLogicalOr(std::vector<std::unique_ptr<LogicalNode>> children);
    std::unique_ptr<LogicalNode> MakeLogicalNot(std::unique_ptr<LogicalNode> child);
    std::unique_ptr<LogicalNode> StealFirstChild(std::unique_ptr<LogicalNode> target);
    std::vector<std::unique_ptr<LogicalNode>> StealChildren(std::unique_ptr<LogicalNode> target);

    /**
     * Logical combination, such as an AND or an OR.
     * @sa LogicalNode
     */
    class LogicalCombination : public LogicalNode
    {
        NodeVector children;
        LogicalNodeType type;
        friend std::unique_ptr<LogicalNode> MakeLogicalAnd(std::vector<std::unique_ptr<LogicalNode>> children);
        friend std::unique_ptr<LogicalNode> MakeLogicalOr(std::vector<std::unique_ptr<LogicalNode>> children);
        friend std::unique_ptr<LogicalNode> MakeLogicalNot(std::unique_ptr<LogicalNode> child);
        friend std::unique_ptr<LogicalNode> StealFirstChild(std::unique_ptr<LogicalNode> target);
        friend std::vector<std::unique_ptr<LogicalNode>> StealChildren(std::unique_ptr<LogicalNode> target);

        /**
         * Initializes a new instance of the LogicalCombination class.
         * @param children_ The children.
         */
        LogicalCombination(std::vector<std::unique_ptr<LogicalNode>> children_, LogicalNodeType type_)
            : children(std::move(children_))
            , type(type_)
        {
            // Make sure that this is a combination type.
            assert(type != LogicalNodeType::LEAF);
            // Make sure that if this is a NOT, that there is only one child.
            assert(type != LogicalNodeType::NOT || children.size() == 1);
        }
    public:

        /**
         * Gets the children of this LogicalCombination.
         * @return The children of this LogicalCombination.
         */
        virtual std::vector<std::unique_ptr<LogicalNode>> const& GetChildren() const override
        {
            return children;
        }

                /**
         * Gets the name of this node type.
         * @return The name of this node.
         */
        virtual std::wstring GetName() const override
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

        /**
         * Makes a deep copy of this instance.
         * @return A copy of this instance.
         */
        virtual std::unique_ptr<LogicalNode> Clone() const override
        {
            std::unique_ptr<LogicalNode> result(new LogicalCombination(this->CloneChildren(), type));
            return result;
        }

        /**
         * Gets the logical type of this node.
         * @return The logical type of this node.
         */
        virtual LogicalNodeType GetType() const override
        {
            return type;
        }
    };

    inline std::unique_ptr<LogicalNode> MakeLogicalAnd(std::vector<std::unique_ptr<LogicalNode>> children)
    {
        std::unique_ptr<LogicalNode> result(new LogicalCombination(std::move(children), LogicalNodeType::AND));
        return result;
    }

    inline std::unique_ptr<LogicalNode> MakeLogicalOr(std::vector<std::unique_ptr<LogicalNode>> children)
    {
        std::unique_ptr<LogicalNode> result(new LogicalCombination(std::move(children), LogicalNodeType::OR));
        return result;
    }

    inline std::unique_ptr<LogicalNode> MakeLogicalNot(std::unique_ptr<LogicalNode> child)
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
            LogicalNode::NodeVector children;
            children.emplace_back(std::move(child));
            std::unique_ptr<LogicalNode> result(new LogicalCombination(std::move(children), LogicalNodeType::NOT));
            return result;
        }
    }

    inline std::unique_ptr<LogicalNode> StealFirstChild(std::unique_ptr<LogicalNode> target)
    {
        assert(target->GetType() != LogicalNodeType::LEAF);
        assert(target->GetChildren().size() >= 1u);
        return std::move(static_cast<LogicalCombination*>(target.get())->children[0]);
    }

    inline std::vector<std::unique_ptr<LogicalNode>> StealChildren(std::unique_ptr<LogicalNode> target)
    {
        assert(target->GetType() != LogicalNodeType::LEAF);
        return std::move(static_cast<LogicalCombination*>(target.get())->children);
    }

    class LogicalLeaf : public LogicalNode
    {
        static const NodeVector childrenVector;
    public:
       /**
         * Gets the logical type of this node.
         * @return The logical type of this node.
         */
        virtual LogicalNodeType GetType() const override
        {
            return LogicalNodeType::LEAF;
        }

        virtual NodeVector const& GetChildren() const override
        {
            return childrenVector;
        }
    };

    inline std::unique_ptr<LogicalNode> ApplyDemorgan(std::unique_ptr<LogicalNode> source)
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
            [](std::unique_ptr<LogicalNode>&& old) { return MakeLogicalNot(std::move(old)); });
        
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

    inline std::unique_ptr<LogicalNode> MakeNegationNormal(std::unique_ptr<LogicalNode> source, std::size_t notCrosses)
    {
        LogicalNodeType currentType = source->GetType();
        bool noFlip = notCrosses % 2 == 0;
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

    inline std::unique_ptr<LogicalNode> MakeNegationNormal(std::unique_ptr<LogicalNode> source)
    {
        return MakeNegationNormal(std::move(source), 0);
    }

    inline bool IsLiteral(LogicalNode const* node)
    {
        auto currentType = node->GetType();
        return currentType == LogicalNodeType::LEAF
            || ((currentType == LogicalNodeType::NOT) && node->GetChildren()[0]->GetType() == LogicalNodeType::LEAF);
    }

    inline std::wstring GetLiteralName(LogicalNode const* node)
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

    inline std::wstring MakeString(LogicalNode const* source)
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
