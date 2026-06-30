#pragma once
#include <typeindex>
#include "doctest/doctest.h"
#include <string>
#include "../Common.h"

template <typename T>
std::type_index typeIndex()
{
	return std::type_index(typeid(T));
}

//works out of the box, but may be ugly and barely readable. Used as fallback only.
template <typename T>
constexpr const char* mangledTypeName()
{
	return typeid(T).name();
}


//Supply implementation for type T to name it, using macro below.
//Default impl doesnt throw any errors, just returns nullptr.
template <typename T>
constexpr const char* typeName()
{
	return nullptr;
}



template <typename T>
constexpr bool typeIsNamed()
{
	return typeName<T>() != nullptr;
}

#define SV_REGTYPENAME(TYPENAME)	template<>									\
									constexpr const char* typeName<TYPENAME>()	\
									{											\
										return #TYPENAME;						\
									};

#define SV_REGTYPENAME_AS(T, TYPENAME)	template<>									\
										constexpr const char* typeName<T>()			\
										{											\
											return TYPENAME;						\
										};

//You only have to register name here if you want runtime lookup by std::index.
class TypeNames
{
public:
	static const char* getTypeName(std::type_index index)
	{
		if (const auto* nameFunction = getNameFunction(index))
		{
			return (*nameFunction)();
		}
		else return nullptr;
	}

	template<typename T>
	static void registerNameFunction()
	{
		std::type_index index = typeIndex<T>();

		//must not be registered
		SV_ASSERT(!getNameFunction(index));

		//function must be real and return something
		bool functionOk = typeName<T>() != nullptr;
		SV_ASSERT(functionOk);

		instance().typeNameFunctions[index] = &typeName<T>;
	}

private:
	DISABLE_COPY_AND_ASSIGNMENT(TypeNames);
	TypeNames() = default;
	static TypeNames& instance()
	{
		static TypeNames inst;
		return inst;
	}

	using TypeNameFunction = const char* (*)();

	static const TypeNameFunction* getNameFunction(std::type_index index)
	{
		return getValue(instance().typeNameFunctions, index);
	}

private:
	std::map<std::type_index, TypeNameFunction> typeNameFunctions;
};

SV_REGTYPENAME(bool);
SV_REGTYPENAME(int);
SV_REGTYPENAME(double);
SV_REGTYPENAME(std::string);
SV_REGTYPENAME_AS(std::vector<bool>, "BoolVec");