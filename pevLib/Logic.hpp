//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once
#include <memory>
#include <cstdint>
#include <vector>
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
        LogicalNode::NodeVector children;
        children.emplace_back(std::move(child));
        std::unique_ptr<LogicalNode> result(new LogicalCombination(std::move(children), LogicalNodeType::NOT));
        return result;
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
}
