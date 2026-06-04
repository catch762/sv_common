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


#define SV_DECL_PTRS(TYPENAME)  using TYPENAME ## Shared          = std::shared_ptr<TYPENAME>;\
                                using TYPENAME ## Weak            = std::weak_ptr  <TYPENAME>;\
                                using TYPENAME ## Unique          = std::unique_ptr<TYPENAME>;\
                                using Const ## TYPENAME ## Shared = std::shared_ptr<const TYPENAME>;\
                                using Const ## TYPENAME ## Weak   = std::weak_ptr<const TYPENAME>;\
                                using Const ## TYPENAME ## Unique = std::unique_ptr<const TYPENAME>;

#define SV_DECL_OPT(TYPENAME)   using TYPENAME ## Opt    = std::optional<TYPENAME>;

#define SV_DECL_ALIASES(TYPENAME) SV_DECL_PTRS(TYPENAME) SV_DECL_OPT(TYPENAME)

#define DELETE_COPY_CONSTRUCTOR(CLASSNAME) CLASSNAME(const CLASSNAME&) = delete;
#define DELETE_ASSIGNMENT_OP(CLASSNAME) CLASSNAME& operator=(const CLASSNAME&) = delete;

#define DISABLE_COPY_AND_ASSIGNMENT(CLASSNAME)   DELETE_COPY_CONSTRUCTOR (CLASSNAME)\
                                                 DELETE_ASSIGNMENT_OP    (CLASSNAME)

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

class ANSICodes //this is to get colored text in terminal
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

template<typename Key, typename Value>
Value* getValuePtr(const std::map<Key, Value> &map, const Key& key)
{
    auto found = map.find(key);
    if (found != map.end()) return &found->second;
    else return nullptr;
}

template<typename Key, typename Value>
const Value* getValue(const std::map<Key, Value> &map, const Key& key)
{
    auto found = map.find(key);
    if (found != map.end()) return &found->second;
    else return nullptr;
}

template<typename Key, typename Value>
const std::optional<Value> getValueOpt(const std::map<Key, Value> &map, const Key& key)
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