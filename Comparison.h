#pragma once
#include <concepts>
#include <array>

//In C++23, afaik, there will be std::arithmetic concept, for now im using my own.
//Accepts all integers, floats, doubles.
template<typename T>
concept SV_Arithmetic = std::integral<T> || std::floating_point<T> && !std::same_as<T, bool>;

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

    return std::abs(a - b) < std::numeric_limits<C>::epsilon() * C(10);
}

//uses fuzzy comparing when needed
template <SV_Arithmetic T, SV_Arithmetic U>
constexpr bool arithmeticEquals(T a, U b)
{
    if constexpr (std::integral<T> && std::integral<U>)
        return a == b;
    else
        return fuzzyEquals(a, b);
}

template<   SV_Arithmetic AType,
            SV_Arithmetic BType,
            size_t ASize,
            size_t BSize >
bool arithmeticArraysEquals( const std::array<AType, ASize>& arrA,
                             const std::array<BType, BSize>& arrB )
{
    if (ASize != BSize) return false;

    for (int i = 0; i < ASize; ++i)
    {
        if (!arithmeticEquals(arrA[i], arrB[i]))
        {
            return false;
        }
    }

    return true;
}