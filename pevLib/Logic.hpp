//          Copyright Billy O'Neal 2013
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
#pragma once
#include <memory>
#include <vector>
#include <string>
#include <boost/noncopyable.hpp>

namespace pevFind
{
    /**
     * Values that represent the type of a logical node.
     */
    enum class LogicalNodeType;

    /**
     * Logical node.
     */
    class LogicalNode;

    /**
     * Logical combination, such as an AND, OR, or NOT.
     * @sa LogicalNode
     */
    class LogicalCombination;

    /**
     * Logical nodes which are not combinations.
     * @sa LogicalNode
     */
    class LogicalLeaf;

        /**
     * Clone children of a LogicalNode.
     * @param source Source from which children shall be cloned.
     * @return A new vector containing clones of the supplied node's children.
     */
    std::vector<std::unique_ptr<LogicalNode>> CloneChildren(LogicalNode const* source);

    /**
     * Makes a logical AND node.
     * @param children The children of the new AND node.
     * @return A new logical AND node with the supplied children.
     */
    std::unique_ptr<LogicalNode> MakeLogicalAnd(std::vector<std::unique_ptr<LogicalNode>> children);

    /**
     * Makes a logical OR node.
     * @param children The children of the new OR node.
     * @return A new logical OR node with the supplied children.
     */
    std::unique_ptr<LogicalNode> MakeLogicalOr(std::vector<std::unique_ptr<LogicalNode>> children);

    /**
     * Makes a logical negation of the supplied node @a child.
     * @param child The child of the new logical NOT.
     * @return If @a child is a logical not node, returns that node's child. Otherwise, returns a new
     *         logical not node whose child is @a child.
     */
    std::unique_ptr<LogicalNode> MakeLogicalNot(std::unique_ptr<LogicalNode> child);

    /**
     * Destroys a logical node and returns the first child of that node, without constructing any
     * new nodes.
     * @param target The logical node for which the child shall be retrieved.
     * @return The first child of @a target.
     */
    std::unique_ptr<LogicalNode> StealFirstChild(std::unique_ptr<LogicalNode> target) throw();

    /**
     * Destroys a logical node and returns the children of that node, without constructing any new
     * elements.
     * @param target The logical node for which children shall be retrieved.
     * @return The children of @a target.
     */
    std::vector<std::unique_ptr<LogicalNode>> StealChildren(std::unique_ptr<LogicalNode> target) throw();

    /**
     * Applies the generalized DeMorgan's theorem to a Logical Node.
     * @param source The source logical tree onto which DeMorgan's theorem is applied.
     *               This parameter must be an AND or OR node, or a NOT followed by AND
     *               or OR node. If this condition is not met the behavior is undefined.
     * @return A new tree constructed from @a source which is equivalent but has DeMorgan's theorem
     *         applied to it.
     */
    std::unique_ptr<LogicalNode> ApplyDemorgan(std::unique_ptr<LogicalNode> source);

    /**
     * Makes the given logical tree into one which is equivalent but is in negation-normal form.
     * @param source The tree to put into negation-normal form.
     * @return A new tree which is equivalent to @source but is in negation-normal form.
     */
    std::unique_ptr<LogicalNode> MakeNegationNormal(std::unique_ptr<LogicalNode> source);

    /**
     * Query if 'node' is a literal; that is, a leaf or a not followed by a leaf.
     * @param node The node to query
     * @return true if @a node is a literal, false if not.
     */
    bool IsLiteral(LogicalNode const* node) throw();

    /**
     * Gets literal name of a LogicalNode if the node is a literal node. (That is, a leaf, or a not followed by leaf)
     * @param node The node for which a name shall be generated.
     * @return The literal name.
     */
    inline std::wstring GetLiteralName(LogicalNode const* node);

    /**
     * Converts the supplied logical tree into a string.
     * @param source The tree from which a string is constructed.
     * @return A string representation of @a source.
     */
    std::wstring MakeString(LogicalNode const* source);
    
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

    class LogicalNode : boost::noncopyable
    {
    public:
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

        /**
         * Gets the children of this node.
         * @return The children of this node.
         */
        virtual std::vector<std::unique_ptr<LogicalNode>> const& GetChildren() const throw() = 0;

        /**
         * Gets the logical type of this node.
         * @return The logical type of this node.
         */
        virtual LogicalNodeType GetType() const throw() = 0;

        /**
         * Destroys an instance of the LogicalNode class.
         */
        virtual ~LogicalNode() throw() {}
    };

    class LogicalCombination : public LogicalNode
    {
        std::vector<std::unique_ptr<LogicalNode>> children;
        LogicalNodeType type;
        friend std::unique_ptr<LogicalNode> MakeLogicalAnd(std::vector<std::unique_ptr<LogicalNode>> children);
        friend std::unique_ptr<LogicalNode> MakeLogicalOr(std::vector<std::unique_ptr<LogicalNode>> children);
        friend std::unique_ptr<LogicalNode> MakeLogicalNot(std::unique_ptr<LogicalNode> child);
        friend std::unique_ptr<LogicalNode> StealFirstChild(std::unique_ptr<LogicalNode> target) throw();
        friend std::vector<std::unique_ptr<LogicalNode>> StealChildren(std::unique_ptr<LogicalNode> target) throw();

        /**
         * Initializes a new instance of the LogicalCombination class.
         * @param children_ The children.
         */
        LogicalCombination(std::vector<std::unique_ptr<LogicalNode>> children_, LogicalNodeType type_);
    public:

        /**
         * Gets the children of this LogicalCombination.
         * @return The children of this LogicalCombination.
         */
        virtual std::vector<std::unique_ptr<LogicalNode>> const& GetChildren() const throw() override;

        /**
         * Gets the name of this node type.
         * @return The name of this node.
         */
        virtual std::wstring GetName() const override;

        /**
         * Makes a deep copy of this instance.
         * @return A copy of this instance.
         */
        virtual std::unique_ptr<LogicalNode> Clone() const override;

        /**
         * Gets the logical type of this node.
         * @return The logical type of this node.
         */
        virtual LogicalNodeType GetType() const throw() override;
    };

    class LogicalLeaf : public LogicalNode
    {
        static const std::vector<std::unique_ptr<LogicalNode>> childrenVector;
    public:
       /**
         * Gets the logical type of this node.
         * @return The logical type of this node.
         */
        virtual LogicalNodeType GetType() const throw() override;

        /**
         * Gets the children of this node.
         * @return The children. (Always an empty vector for LogicalLeaf instances)
         */
        virtual std::vector<std::unique_ptr<LogicalNode>> const& GetChildren() const throw() override;
    };
}
