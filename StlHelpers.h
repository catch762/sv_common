#pragma once
#include <vector>

template<typename ValueT>
bool vectorContentEquals(const std::vector<ValueT>& a, const std::vector<ValueT>& b)
{
    if (a.size() != b.size()) return false;

    for (int i = 0; i < a.size(); ++i)
    {
        if (a[i] != b[i]) return false;
    }

    return true;
}