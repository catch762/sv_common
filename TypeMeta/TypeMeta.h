#pragma once
#include <typeindex>
#include "doctest/doctest.h"
#include <string>
#include "../Logging.h"

//works out of the box, but may be ugly and barely readable. Used as fallback.
template <typename T>
constexpr const char* mangledTypeName()
{
	return typeid(T).name();
}


//later use std::type_index, int is for transition
using TypeId = int;

template<typename T>
class TypeMeta
{
public:
	static TypeId id()
	{
		static TypeId id = static_cast<TypeId>(typeid(T).hash_code());
		return id;
	}

	static const char* name()
	{
		static_assert(false, "unimplemented!");
		return nullptr;
	}
};

#define SV_REGTYPE(TYPENAME)	template<>												\
								class TypeMeta<TYPENAME>								\
								{														\
								public:													\
									static const char* name() { return #TYPENAME; }		\
								};

SV_REGTYPE(bool);

template <typename T>
inline TypeId typeId()
{
	return TypeMeta<T>::id();
}

template <typename T>
inline const char* typeName()
{
	return TypeMeta<T>::name();
}