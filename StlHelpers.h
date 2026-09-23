#pragma once
#include <vector>
#include <array>
#include "Common.h"

template <typename VectorT>
concept IsStdVector = std::same_as<
    std::decay_t<VectorT>,
    std::vector<typename std::decay_t<VectorT>::value_type, typename std::decay_t<VectorT>::allocator_type>
>;

// usage: using TypeInt = getVectorElementType<decltype( std::vector<int>{} )>;
template <IsStdVector VectorT>
using getVectorElementType = typename std::decay_t<VectorT>::value_type;

template<typename Key, typename Value, typename Compare>
Value* getValuePtr(const std::map<Key, Value, Compare>& map, const Key& key)
{
    auto found = map.find(key);
    if (found != map.end()) return &found->second;
    else return nullptr;
}

template<typename Key, typename Value, typename Compare>
const Value* getValue(const std::map<Key, Value, Compare>& map, const Key& key)
{
    auto found = map.find(key);
    if (found != map.end()) return &found->second;
    else return nullptr;
}

template<typename Key, typename Value, typename Compare>
std::optional<Value> getValueOpt(const std::map<Key, Value, Compare>& map, const Key& key)
{
    if (auto value = getValue(map, key)) return *value;
    else return {};
}

//Appends 'source' to 'destination', moving it. So 'source' is left in undefined state after this.
template<typename T>
void moveVectorToTheEndOfOther(std::vector<T>& destination, std::vector<T>& source)
{
    destination.insert(
        destination.end(),
        std::make_move_iterator(source.begin()),
        std::make_move_iterator(source.end())
    );
}