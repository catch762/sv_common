#pragma once
#include <any>
#include <typeindex>
#include <optional>
#include "Common.h"
#include "Formatters.h"
#include "TypeMeta/TypeNaming.h"

using anyOpt = std::optional<std::any>;

//note: if any is empty, it will return std::type_index(typeid(void))
inline std::type_index typeIndex(const std::any& any)
{
	return std::type_index(any.type());
}

inline const char* anyTypeName(const std::any& any)
{
	return TypeNames::getTypeName(typeIndex(any));
}

//this will return true for two empty std::any's
inline bool anyHoldSameType(const std::any& first, const std::any& second)
{
	return typeIndex(first) == typeIndex(second);
}
inline bool anyHoldSameType(const std::any* first, const std::any* second)
{
	SV_ASSERT(first);
	SV_ASSERT(second);
	return anyHoldSameType(*first, *second);
}

inline std::string anyInfo(const std::any& any)
{
	if (!any.has_value())
	{
		return "any_empty[]";
	}
	else if (auto* myTypeName = anyTypeName(any))
	{
		return std::format("any_named[{}]", myTypeName);
	}
	else
	{
		//prints mangled name
		return std::format("any_unnamed[{}]", any.type().name());
	}
}

template <typename T>
bool anyHoldsType(const std::any& any)
{
	return any.type() == typeid(T);
}

template <typename T>
const T* anyGet(const std::any& any)
{
	return std::any_cast<T>(&any);
}

template <typename T>
T* anyGet(std::any& any)
{
	return std::any_cast<T>(&any);
}

template <typename T>
std::optional<T> anyGetOpt(const std::any& any)
{
	if (auto* val = anyGet<T>(any))
	{
		return std::optional<T>(*val);
	}
	else return {};
}

SV_DECL_STD_FORMATTER(std::any, anyInfo(obj));