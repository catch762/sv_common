#include "BasicToken.h"

BasicTokenType BasicToken::type() const
{
    SV_ASSERT(!data.valueless_by_exception()); //it never happens, can delete it
    return BasicTokenType(data.index());
}

bool BasicToken::isSymbol           () const { return type() == BasicTokenType::Symbol; }
bool BasicToken::isString           () const { return type() == BasicTokenType::String; }
bool BasicToken::isSpecialCharachter() const { return type() == BasicTokenType::SpecialCharachter; }
bool BasicToken::isNumberInt        () const { return type() == BasicTokenType::NumberInt; }
bool BasicToken::isNumberDouble     () const { return type() == BasicTokenType::NumberDouble; }
bool BasicToken::isNumber           () const { return isNumberInt() || isNumberDouble(); }

std::string BasicToken::dataToString()
{
    return std::visit([](auto &&arg) -> std::string
    {
        return std::format("{}", arg);
    }, data);
}
