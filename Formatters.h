#pragma once
#include <format>

template <typename OutputIt>
void writeStr(OutputIt& out, std::string_view str) {
    out = std::ranges::copy(str, out).out;
}

//***************************************************************
// Example usage:
//
//      class YourCustomType
//      {
//          std::string toString(){return "...";}
//      }
//
//      SV_DECL_STD_FORMATTER(YourCustomType, obj.toString());
//
// Then you can do std::print, std::format etc:
//
//      YourCustomType val;
//      std::print("Hello, val is {}", val);
//***************************************************************
#define SV_DECL_STD_FORMATTER(TYPENAME, EXPRESSION_THAT_MAKES_STDSTRING_FROM_OBJ)                   \
    template <>                                                                                     \
    struct std::formatter<TYPENAME> {                                                               \
        constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }                \
        auto format(const TYPENAME& obj, std::format_context& ctx) const {                          \
            return std::format_to(ctx.out(), "{}", (EXPRESSION_THAT_MAKES_STDSTRING_FROM_OBJ) );    \
        }                                                                                           \
    };



//******************************************************************
// [ Containers: quick and dirty formatting impl ]
//
// As long as you have defined formatter for type T, with these, you 
// should be able to do std::format({}, container_of_type_T).
//
// I have no fucking clue why this doesnt exist in C++ by default.
//
// This implementation also overrides the way vectors with default
// types would print itself, btw.
// And my implementation probably ignores proper context parsing -
// thats a TODO.
//******************************************************************

//todo remove inheritance lol

template <typename T>
struct std::formatter<std::vector<T>> : std::formatter<T> {

    std::string_view left = "[ ";
    std::string_view right = " ]";
    std::string_view sep = ", ";

    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatCtx>
    auto format(const std::vector<T>& vec, FormatCtx& fctx) const
    {
        auto out = fctx.out();

        writeStr(out, left);

        for (size_t i = 0; i < vec.size(); ++i)
        {
            out = std::formatter<T>::format(vec[i], fctx);
            if (i < vec.size() - 1)
            {
                writeStr(out, sep);
            }
        }
        writeStr(out, right);
        return out;
    }
};

template <typename T, size_t Count>
struct std::formatter<std::array<T, Count>> : std::formatter<T> {

    using ArrT = std::array<T, Count>;

    std::string_view left = "[ ";
    std::string_view right = " ]";
    std::string_view sep = ", ";

    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatCtx>
    auto format(const ArrT& arr, FormatCtx& fctx) const
    {
        auto out = fctx.out();

        writeStr(out, left);

        for (size_t i = 0; i < arr.size(); ++i)
        {
            out = std::formatter<T>::format(arr[i], fctx);
            if (i < arr.size() - 1)
            {
                writeStr(out, sep);
            }
        }
        writeStr(out, right);
        return out;
    }
};

template <typename T>
struct std::formatter<std::set<T>> : std::formatter<T> {

    std::string_view left = "[ ";
    std::string_view right = " ]";
    std::string_view sep = ", ";

    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatCtx>
    auto format(const std::set<T>& set, FormatCtx& fctx) const
    {
        auto out = fctx.out();

        writeStr(out, left);

        int i = 0;
        for (const auto& val : set)
        {
            out = std::formatter<T>::format(val, fctx);
            if (i < set.size() - 1)
            {
                writeStr(out, sep);
            }

            i++;
        }
        writeStr(out, right);
        return out;
    }
};

//****************
// [ /Containers ]
//****************

template <typename A, typename B>
struct std::formatter<std::pair<A, B>> {

    std::string_view left = "[";
    std::string_view right = "]";
    std::string_view sep = ", ";

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatCtx>
    auto format(const std::pair<A, B>& p, FormatCtx& fctx) const
    {
        auto out = fctx.out();

        for (char c : left) *out++ = c;
        out = std::formatter<A>{}.format(p.first, fctx);
        for (char c : sep) *out++ = c;
        out = std::formatter<B>{}.format(p.second, fctx);
        for (char c : right) *out++ = c;

        return out;
    }
};