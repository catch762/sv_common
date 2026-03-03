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

#define SV_DECL_PTRS(TYPENAME)  using TYPENAME ## Shared = std::shared_ptr<TYPENAME>;\
                                using TYPENAME ## Weak   = std::weak_ptr  <TYPENAME>;\
                                using TYPENAME ## Unique = std::unique_ptr<TYPENAME>;

#define SV_DECL_OPT(TYPENAME)   using TYPENAME ## Opt    = std::optional<TYPENAME>;

#define SV_DECL_ALIASES(TYPENAME) SV_DECL_PTRS(TYPENAME) SV_DECL_OPT(TYPENAME)


SV_DECL_OPT(int)

SV_DECL_OPT(QString)
SV_DECL_OPT(QJsonArray)

inline QStringOpt jsonGetStringOpt(const QJsonObject& json, const QString& key)
{
    auto value = json[key];
    if (value.isString()) return value.toString();
    else return {};
};

inline QJsonArrayOpt jsonGetArrayOpt(const QJsonObject& json, const QString& key)
{
    auto value = json[key];
    if (value.isArray()) return value.toArray();
    else return {};
};

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