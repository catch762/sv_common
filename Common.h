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
#include <cassert>
#include <cmath>
#include <variant>
#include <string_view>
#include <algorithm>

// The MSVC Fix: Wrap the macro call in an outer expansion macro to unpack __VA_ARGS__
#define SV_EXP(x) x

#define SV_DECL_PTRS(TYPENAME)  using TYPENAME ## Shared          = std::shared_ptr<TYPENAME>;\
                                using TYPENAME ## Weak            = std::weak_ptr  <TYPENAME>;\
                                using TYPENAME ## Unique          = std::unique_ptr<TYPENAME>;\
                                using Const ## TYPENAME ## Shared = std::shared_ptr<const TYPENAME>;\
                                using Const ## TYPENAME ## Weak   = std::weak_ptr<const TYPENAME>;\
                                using Const ## TYPENAME ## Unique = std::unique_ptr<const TYPENAME>;

#define SV_DECL_OPT(TYPENAME)   using TYPENAME ## Opt    = std::optional<TYPENAME>;

#define SV_DECL_ERR(TYPENAME)   using TYPENAME ## OrError    = std::variant<TYPENAME, std::string /*errorstring*/>;

template<typename Whatever>
inline const std::string* getError(const std::variant<Whatever, std::string>& somethingOrError)
{
    return std::get_if<std::string>(&somethingOrError);
}

template<typename Whatever>
inline Whatever* getValue(std::variant<Whatever, std::string>& somethingOrError)
{
    return std::get_if<Whatever>(&somethingOrError);
}

template<typename WhateverVariant>
inline std::string variantToString(const WhateverVariant& variantOfWhatever)
{
    return std::visit([](auto &&val){return std::format("{}", val);}, variantOfWhatever);
}

#define SV_DECL_ALIASES(TYPENAME) SV_DECL_PTRS(TYPENAME) SV_DECL_OPT(TYPENAME) SV_DECL_ERR(TYPENAME)

#define DELETE_COPY_CONSTRUCTOR(CLASSNAME) CLASSNAME(const CLASSNAME&) = delete;
#define DELETE_ASSIGNMENT_OP(CLASSNAME) CLASSNAME& operator=(const CLASSNAME&) = delete;

#define DISABLE_COPY_AND_ASSIGNMENT(CLASSNAME)   DELETE_COPY_CONSTRUCTOR (CLASSNAME)\
                                                 DELETE_ASSIGNMENT_OP    (CLASSNAME)

SV_DECL_ALIASES(int)
SV_DECL_ALIASES(bool)
SV_DECL_ALIASES(double)
SV_DECL_ALIASES(char)

using StringSet = std::set<std::string>;
SV_DECL_ALIASES(StringSet);

using StringErrOpt = std::optional<std::string>;

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

class ANSICodes //this is to get colored text in terminal
{
public:
    // Colors
    static inline constexpr std::string_view black   = "\033[30m";
    static inline constexpr std::string_view red     = "\033[31m";
    static inline constexpr std::string_view orange  = "\033[38;5;208m";
    static inline constexpr std::string_view green   = "\033[32m";
    static inline constexpr std::string_view yellow  = "\033[33m";
    static inline constexpr std::string_view blue    = "\033[34m";
    static inline constexpr std::string_view magenta = "\033[35m";
    static inline constexpr std::string_view cyan    = "\033[36m";
    static inline constexpr std::string_view white   = "\033[37m";

    // Styles
    static inline constexpr std::string_view bold      = "\033[1m";
    static inline constexpr std::string_view dim       = "\033[2m";
    static inline constexpr std::string_view italic    = "\033[3m";
    static inline constexpr std::string_view underline = "\033[4m";
    static inline constexpr std::string_view blink     = "\033[5m";
    static inline constexpr std::string_view reset     = "\033[0m";
    static inline constexpr std::string_view none      = "";

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

inline std::string makeInvalidIndexError(intOpt index, int itemsCount, const char* contextString = nullptr)
{
    return std::format("[{}]: invalid index [{}] to access [{}] items",
                        contextString ? contextString : "",
                        index ? std::to_string(*index) : "nullopt",
                        itemsCount);
}

inline StringErrOpt getErrorIfInvalidIndex(intOpt index, int itemsCount, const char* contextString = nullptr)
{
    if (isValidIndex(index, itemsCount))
    {
        return {};
    }
    else return makeInvalidIndexError(index, itemsCount, contextString);
}

template<typename Key, typename Value, typename Compare>
Value* getValuePtr(const std::map<Key, Value, Compare> &map, const Key& key)
{
    auto found = map.find(key);
    if (found != map.end()) return &found->second;
    else return nullptr;
}

template<typename Key, typename Value, typename Compare>
const Value* getValue(const std::map<Key, Value, Compare> &map, const Key& key)
{
    auto found = map.find(key);
    if (found != map.end()) return &found->second;
    else return nullptr;
}

template<typename Key, typename Value, typename Compare>
std::optional<Value> getValueOpt(const std::map<Key, Value, Compare> &map, const Key& key)
{
    if (auto value = getValue(map, key)) return *value;
    else return {};
}

/*
template<class... Ts>
struct sv_overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
sv_overloaded(Ts...) -> sv_overloaded<Ts...>;
*/

template <typename VectorT>
concept IsStdVector = std::same_as<
    std::decay_t<VectorT>,
    std::vector<typename std::decay_t<VectorT>::value_type, typename std::decay_t<VectorT>::allocator_type>
>;
// usage: using TypeInt = getVectorElementType<decltype( std::vector<int>{} )>;
template <IsStdVector VectorT>
using getVectorElementType = typename std::decay_t<VectorT>::value_type;

//Appends 'source' to 'destination', moving it. So 'source' is left in undefined state after this.
template<typename T>
inline void moveVectorToTheEndOfOther(std::vector<T>& destination, std::vector<T>& source)
{
    destination.insert(
      destination.end(),
      std::make_move_iterator(source.begin()),
      std::make_move_iterator(source.end())
    );
}

//Accepts floats and doubles
template <std::floating_point T, std::floating_point U>
constexpr bool fuzzyEquals(T _a, U _b)
{
    using C = std::common_type_t<T, U>; //returns bigger type
    const C a = static_cast<C>(_a);
    const C b = static_cast<C>(_b);

    //Two questionable checks:
    if (std::isnan(a) || std::isnan(b)) return false; 
    if (std::isinf(a) || std::isinf(b)) return a == b; //there are +inf and -inf

    return std::abs(a - b) < std::numeric_limits<C>::epsilon() * C(100);
}

//In C++23, afaik, there will be std::arithmetic concept, for now im using my own.
//Accepts all integers, floats, doubles.
template<typename T>
concept SV_Arithmetic = std::integral<T> || std::floating_point<T> && !std::same_as<T, bool>;

//uses fuzzy comparing when needed
template <SV_Arithmetic T, SV_Arithmetic U>
constexpr bool arithmeticEquals(T a, U b)
{
    if constexpr (std::integral<T> && std::integral<U>)
        return a == b;
    else
        return fuzzyEquals(a, b);
}

//Assumes base 10, expects strictly valid number strings, no trailing spacebars or anything - otherwise false is returned.
//Takes minus symbol into account, ignores plus.
//
inline bool stringIsValidIntOrDouble(const std::string& text, bool& isInt)
{
    bool hasPoint = false;

    for (int i = 0; i < text.size(); ++i)
    {
        const auto ch       = text[i];
        
        const auto isDigit  = std::isdigit(ch);
        const auto isMinus  = ch == '-';
        const auto isPlus   = ch == '+';
        const auto isPoint  = ch == '.';

        const auto isFirstChar = i == 0;
        const auto isLastChar = i == text.size()-1;

        if (isPoint) hasPoint = true;

        const auto isValid = isDigit ||
                            (isMinus && isFirstChar) ||
                            (isPlus  && isFirstChar) ||
                            (isPoint && (!isFirstChar && !isLastChar));
        if(!isValid) return false;
    }

    isInt = !hasPoint;
    return true;
}

inline std::optional<std::variant<int, double>> convertStringToIntOrDouble(const std::string& text)
{
    bool isInt;
    if (stringIsValidIntOrDouble(text, isInt))
    {
        //I assume that now conversion should always work without error.
        //I cant just use strtod directly - because i dont want fucking "NaN" string to be parsed as
        //"valid" double containing, you guessed it, NaN. Etc. There are many things like that in strtod implementation.

        char* endptr = nullptr;
        if (isInt)
        {
            auto value        = std::strtol(text.c_str(), &endptr, 10);
            bool fuckingError = endptr == text.c_str();
            assert(!fuckingError);
            return int(value);
        }
        else
        {
            auto value        = std::strtod(text.c_str(), &endptr);
            bool fuckingError = endptr == text.c_str();
            assert(!fuckingError);
            return value;
        }
    }
    else return {};
}