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

// Macro expansion. Needed for MSVC most of all?
#define SV_EXP(x) x

#define SV_DECL_PTRS(TYPENAME)  using TYPENAME ## Shared          = std::shared_ptr<TYPENAME>;\
                                using TYPENAME ## Weak            = std::weak_ptr  <TYPENAME>;\
                                using TYPENAME ## Unique          = std::unique_ptr<TYPENAME>;\
                                using Const ## TYPENAME ## Shared = std::shared_ptr<const TYPENAME>;\
                                using Const ## TYPENAME ## Weak   = std::weak_ptr  <const TYPENAME>;\
                                using Const ## TYPENAME ## Unique = std::unique_ptr<const TYPENAME>;

#define SV_DECL_OPT(TYPENAME)   using TYPENAME ## Opt    = std::optional<TYPENAME>;

template<typename ActualValue>
using ValueOrError = std::variant<ActualValue, std::string>;

#define SV_DECL_ERR(TYPENAME)   using TYPENAME ## OrError    = ValueOrError<TYPENAME>;

template<typename ActualValue>
const std::string* getError(const ValueOrError<ActualValue>& valueOrError)
{
    return std::get_if<std::string>(&valueOrError);
}
template<typename ActualValue>
ActualValue* getValue(ValueOrError<ActualValue>& valueOrError)
{
    return std::get_if<ActualValue>(&valueOrError);
}

template<typename WhateverVariant>
std::string variantToString(const WhateverVariant& variantOfWhatever)
{
    return std::visit([](auto &&val){return std::format("{}", val);}, variantOfWhatever);
}

#define SV_DECL_ALIASES(TYPENAME) SV_DECL_PTRS(TYPENAME) SV_DECL_OPT(TYPENAME) SV_DECL_ERR(TYPENAME)

#define DELETE_COPY_CONSTRUCTOR(CLASSNAME)  CLASSNAME(const CLASSNAME&) = delete;
#define DELETE_ASSIGNMENT_OP(CLASSNAME)     CLASSNAME& operator=(const CLASSNAME&) = delete;

#define DISABLE_COPY_AND_ASSIGNMENT(CLASSNAME)   DELETE_COPY_CONSTRUCTOR (CLASSNAME)\
                                                 DELETE_ASSIGNMENT_OP    (CLASSNAME)

SV_DECL_ALIASES(int)
SV_DECL_ALIASES(bool)
SV_DECL_ALIASES(double)
SV_DECL_ALIASES(float)
SV_DECL_ALIASES(char)

using StringSet = std::set<std::string>;
SV_DECL_ALIASES(StringSet);

//Here 'nullopt' means 'no error'
using StringErrOpt = std::optional<std::string>;

template<typename T> 
T* removeConst(const T* ptr)
{ 
    return const_cast<T*>(ptr);
}

template<typename T>
const T* asConst(T* ptr)
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

    static std::string rgb(uint8_t r, uint8_t g, uint8_t b)
    {
        return std::format("\033[38;2;{};{};{}m", r, g, b);
    }
};



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