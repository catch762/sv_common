#include "AnyHelpers.h"

std::type_index typeIndex(const std::any& any)
{
	return std::type_index(any.type());
}

const char* anyTypeName(const std::any& any)
{
	return TypeNames::getTypeName(typeIndex(any));
}

std::string anyTypeNameOrMangled(const std::any& any)
{
	if (auto userName = TypeNames::getTypeName(typeIndex(any)))
	{
		return userName;
	}
	else return any.type().name();
}

//this will return true for two empty std::any's
bool anyHoldSameType(const std::any& first, const std::any& second)
{
	return typeIndex(first) == typeIndex(second);
}
bool anyHoldSameType(const std::any* first, const std::any* second)
{
	SV_ASSERT(first);
	SV_ASSERT(second);
	return anyHoldSameType(*first, *second);
}

std::string anyInfo(const std::any& any)
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