#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <format>
#include <iostream>
#include <fstream>
#include <chrono>
#include <utility>


#define SV_DECL_PTRS(TYPENAME)  using TYPENAME ## Shared = std::shared_ptr<TYPENAME>;\
                                using TYPENAME ## Weak   = std::weak_ptr  <TYPENAME>;\
                                using TYPENAME ## Unique = std::unique_ptr<TYPENAME>;

#define SV_DECL_OPT(TYPENAME)   using TYPENAME ## Opt    = std::optional<TYPENAME>;

#define SV_DECL_ALIASES(TYPENAME) SV_DECL_PTRS(TYPENAME) SV_DECL_OPT(TYPENAME)


SV_DECL_OPT(int)
SV_DECL_OPT(bool)
SV_DECL_OPT(double)


template<typename T> 
inline T* removeConst(const T* ptr)
{ 
    return const_cast<T*>(ptr);
}

template<typename T>
inline const T* asConst(T* ptr)
{ 
    return static_cast<const T*>(ptr);
}

std::string getCurrentTimeHMS();

class ANSICodes
{
public:
    // Colors
    static inline constexpr std::string black   = "\033[30m";
    static inline constexpr std::string red     = "\033[31m";
    static inline constexpr std::string orange  = "\033[38;5;208m";
    static inline constexpr std::string green   = "\033[32m";
    static inline constexpr std::string yellow  = "\033[33m";
    static inline constexpr std::string blue    = "\033[34m";
    static inline constexpr std::string magenta = "\033[35m";
    static inline constexpr std::string cyan    = "\033[36m";
    static inline constexpr std::string white   = "\033[37m";

    // Styles
    static inline constexpr std::string bold      = "\033[1m";
    static inline constexpr std::string dim       = "\033[2m";
    static inline constexpr std::string italic    = "\033[3m";
    static inline constexpr std::string underline = "\033[4m";
    static inline constexpr std::string blink     = "\033[5m";
    static inline constexpr std::string reset     = "\033[0m";
    static inline constexpr std::string none      = "";

    static inline std::string rgb(uint8_t r, uint8_t g, uint8_t b)
    {
        return std::format("\033[38;2;{};{};{}m", r, g, b);
    }
};


template<typename T>
inline T mix(T a, T b, double b_ratio01)
{
    return a + (b - a) * b_ratio01;
}

inline double value11To01(double value11u)
{
    auto value01u = (value11u + 1.0) * 0.5;
    return std::clamp(value01u, 0.0, 1.0);
}

inline double value01To11(double value01u)
{
    auto value11u = value01u * 2.0 - 1.0;
    return std::clamp(value11u, -1.0, 1.0);
}

//returns 0 if value is at left, returns 1 if value is at right. Result clamped to [0,1]
inline double getValue01Clamped(double value, double left, double right)
{
    double span = right-left;

    if (abs(span) < (std::numeric_limits<double>::epsilon() * 100.0))
    {
        return 0.0;
    }

    auto res = std::clamp((value-left) / span, 0.0, 1.0);

    return res;
}

inline double getValue11Clamped(double value, double left, double right)
{
    return value01To11( getValue01Clamped(value, left, right) );
}

inline bool isValidIndex(intOpt index, int itemsCount)
{
    return index && *index >= 0 && *index < itemsCount;
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
    