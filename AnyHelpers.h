#pragma once
#include <any>
#include <typeindex>

//note: if any is empty, it will return std::type_index(typeid(void))
inline std::type_index typeIndex(const std::any& any)
{
	return std::type_index(any.type());
}