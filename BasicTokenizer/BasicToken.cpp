#include "BasicToken.h"

BasicToken::BasicToken()
{
    setSymbolData("");
}

BasicToken BasicToken::makeSymbol(std::string data)
{
    BasicToken token;
    token.setSymbolData(std::move(data));
    return token;
}

BasicToken BasicToken::makeString(std::string data)
{
    BasicToken token;
    token.setTypeAndData<BasicTokenType::String>(std::move(data));
    return token;
}

BasicToken BasicToken::makeSpecialCharachter(char ch)
{
    BasicToken token;
    token.setTypeAndData<BasicTokenType::SpecialCharachter>(ch);
    return token;
}

BasicToken BasicToken::makeNumberInt(int number)
{
    BasicToken token;
    token.setTypeAndData<BasicTokenType::NumberInt>(number);
    return token;
}

BasicToken BasicToken::makeNumberDouble(double number)
{
    BasicToken token;
    token.setTypeAndData<BasicTokenType::NumberDouble>(number);
    return token;
}

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

std::string BasicToken::dataToString() const
{
    return std::visit([](auto &&arg) -> std::string
    {
        return std::format("{}", arg);
    }, data);
}

std::string BasicToken::info() const
{
    return std::format("{}[{}]", basicTokenTypeToString(type()), dataToString());
}

template<typename T> 
const T& returnDefaultValAndLogError(BasicTokenType expectedType, BasicTokenType actualType)
{
    SV_ERROR(std::format("Trying to get data from token expecting it to be type [{}] but its [{}]",
                         int(expectedType), int(actualType) ));

    static const T defaultVal = {};
    return defaultVal;
}

const std::string &BasicToken::getSymbolData() const
{
    if (isSymbol())
    {
        return std::get<static_cast<int>(BasicTokenType::Symbol)>(data);
    }
    else return returnDefaultValAndLogError<std::string>(BasicTokenType::Symbol, type());
}

const std::string &BasicToken::getStringData() const
{
    if (isString())
    {
        return std::get<static_cast<int>(BasicTokenType::String)>(data);
    }
    else return returnDefaultValAndLogError<std::string>(BasicTokenType::String, type());
}

char BasicToken::getSpecialCharachterData() const
{
    if (isSpecialCharachter())
    {
        return std::get<static_cast<int>(BasicTokenType::SpecialCharachter)>(data);
    }
    else return returnDefaultValAndLogError<char>(BasicTokenType::SpecialCharachter, type());
}

int BasicToken::getNumberIntData() const
{
    if (isNumberInt())
    {
        return std::get<static_cast<int>(BasicTokenType::NumberInt)>(data);
    }
    else return returnDefaultValAndLogError<int>(BasicTokenType::NumberInt, type());
}

double BasicToken::getNumberDoubleData() const
{
    if (isNumberDouble())
    {
        return std::get<static_cast<int>(BasicTokenType::NumberDouble)>(data);
    }
    else return returnDefaultValAndLogError<double>(BasicTokenType::NumberDouble, type());
}

void BasicToken::setSymbolData(std::string data)
{
    setTypeAndData<BasicTokenType::Symbol>(std::move(data));
}

void BasicToken::setStringData(std::string data)
{
    setTypeAndData<BasicTokenType::String>(std::move(data));
}

void BasicToken::setSpecialCharachterData(char ch)
{
    setTypeAndData<BasicTokenType::SpecialCharachter>(ch);
}

void BasicToken::setNumberIntData(int num)
{
    setTypeAndData<BasicTokenType::NumberInt>(num);
}

void BasicToken::setNumberDoubleData(double num)
{
    setTypeAndData<BasicTokenType::NumberDouble>(num);
}

const char* basicTokenTypeToString(BasicTokenType type)
{
    const int entriesCount = int(BasicTokenType::Last) + 1;
    static const char* names[entriesCount] = {
        "Symbol",
        "String",
        "Char",
        "Int",
        "Double",
    };

    int idx = int(type);
    if (idx >= 0 && idx < entriesCount) return names[idx];
    else
    {
        SV_ERROR(std::format("Bad BasicTokenType enum value outside range: {}", idx));
        return nullptr;
    }
}

bool BasicToken::operator==(const BasicToken &other) const
{
    if (type() != other.type()) return false;

         if (isSymbol           ()) return getSymbolData            () == other.getSymbolData();
    else if (isString           ()) return getStringData            () == other.getStringData();
    else if (isSpecialCharachter()) return getSpecialCharachterData () == other.getSpecialCharachterData();
    else if (isNumberInt        ()) return getNumberIntData         () == other.getNumberIntData();
    else if (isNumberDouble     ()) return abs(getNumberDoubleData  () - other.getNumberDoubleData()) < 0.00000001;
    else SV_UNREACHABLE();
}