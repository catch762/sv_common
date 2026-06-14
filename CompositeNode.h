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

    static CompositeNode<LeafType> makeComposite(Children children = {})
    {
        return CompositeNode<LeafType>(std::move(children));
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

    /*std::string toString() const
    {
        return isLeaf() ? toStringInternal() : "\n" + toStringInternal();
    }*/

    std::string toString(int level = 0) const
    {
        auto ind = std::string((level) * 3, ' ');
        if (isLeaf())
        {
            return ind + std::format("{}", *getLeafValue());
        }
        else if (auto* children = getChildren())
        {
            std::string res = ind + "{\n";
            for (int i = 0; i < children->size(); ++i)
            {
                res += (*children)[i].toString(level + 1);
                if (i != children->size()-1) res += ",";
                res += "\n";
            }
            res += ind + "}";
            return res;
        }
        else SV_UNREACHABLE();
    }

    //assumes u can compare LeafValue's
    bool operator==(const CompositeNode& other) const
    {
        if (isLeaf() != other.isLeaf()) return false;

        if (isLeaf())
        {
            return *getLeafValue() == *other.getLeafValue();
        }
        else
        {
            const auto* thisChildren  = getChildren();
            const auto* otherChildren = other.getChildren();

            if (thisChildren->size() != otherChildren->size()) return false;

            for (int i = 0; i < thisChildren->size(); ++i)
            {
                const auto &thisChild  = (*thisChildren)[i];
                const auto &otherChild = (*otherChildren)[i];

                if (thisChild != otherChild) return false;
            }

            //all children equal, so
            return true;
        }
    }

private:
    std::variant<LeafType, Children> data;
};


template <typename LeafType>
struct std::formatter<CompositeNode<LeafType>>
{
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const CompositeNode<LeafType>& obj, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", obj.toString() );
    }
};