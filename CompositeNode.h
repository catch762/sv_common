#pragma once
#include "Common.h"

//***************************************************************
//
// Small simple class for Composite trees, which means
// node either holds a LeafType value, or a list of child nodes.
//
//***************************************************************
template<typename LeafType>
struct CompositeNode
{
public:
    using Children = std::vector<CompositeNode>;

    CompositeNode() = delete;

    CompositeNode(LeafType leafValue) 
        : data(std::move(leafValue))
    {
    }

    CompositeNode(Children children)
        : data(std::move(children))
    {
    }

    bool isLeaf() const
    {
        return std::holds_alternative<LeafType>(data);
    }
    bool isComposite() const
    {
        return std::holds_alternative<Children>(data);
    }

    // These return nullptr if assumed type is wrong
    LeafType* getLeafValue()
    {
        return std::get_if<LeafType>(&data);
    }
    Children* getChildren()
    {
        return std::get_if<Children>(&data);
    }
    const LeafType* getLeafValue() const
    {
        return std::get_if<LeafType>(&data);
    }
    const Children* getChildren() const
    {
        return std::get_if<Children>(&data);
    }

private:
    std::variant<LeafType, Children> data;
};