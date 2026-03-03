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

#define SV_DECL_PTRS(TYPENAME)  using TYPENAME ## Shared = std::shared_ptr<TYPENAME>;\
                                using TYPENAME ## Weak   = std::weak_ptr  <TYPENAME>;\
                                using TYPENAME ## Unique = std::unique_ptr<TYPENAME>;

#define SV_DECL_OPT(TYPENAME)   using TYPENAME ## Opt    = std::optional<TYPENAME>;

#define SV_DECL_ALIASES(TYPENAME) SV_DECL_PTRS(TYPENAME) SV_DECL_OPT(TYPENAME)


SV_DECL_OPT(int)


//todo delete:
// if you want to have same function in two versions:   - T function();
//                                                      - const T function() const;
// And want to avoid code duplication, define const one first, then add:
// 'T function() { return SV_STRIP_CONST(T, function() ); }
//
// Compiler is guaranteed to choose const version for inner call, apparently.


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