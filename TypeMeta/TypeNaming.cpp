#include "TypeNaming.h"
#include "../StlHelpers.h"

const char* TypeNames::getTypeName(std::type_index index)
{
	if (const auto* nameFunction = getNameFunction(index))
	{
		return (*nameFunction)();
	}
	else return nullptr;
}

TypeNames& TypeNames::instance()
{
	static TypeNames inst;
	return inst;
}

const TypeNames::TypeNameFunction* TypeNames::getNameFunction(std::type_index index)
{
	return getValue(instance().typeNameFunctions, index);
}