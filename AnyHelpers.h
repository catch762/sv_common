#pragma once
#include <any>
#include <typeindex>
#include <optional>
#include "Common.h"
#include "Formatters.h"
#include "TypeMeta/TypeNaming.h"

using anyOpt = std::optional<std::any>;

//note: if any is empty, it will return std::type_index(typeid(void))
std::type_index typeIndex(const std::any& any);

const char* anyTypeName(const std::any& any);

std::string anyTypeNameOrMangled(const std::any& any);

//this will return true for two empty std::any's
bool anyHoldSameType(const std::any& first, const std::any& second);
bool anyHoldSameType(const std::any* first, const std::any* second);

std::string anyInfo(const std::any& any);

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