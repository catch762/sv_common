#pragma once
#include <vector>
#include <array>
#include "Common.h"

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