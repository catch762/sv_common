#pragma once
#include "sv_common.h"

enum class BasicTokenType : int
{
    Symbol = 0, //Most generic type, just some text.
    String,
    SpecialCharachter,
    NumberInt,
    NumberDouble,

    Last = NumberDouble
};

//BasicTokenType is used as index.
using BasicTokenDataVariant = std::variant<std::string,
                                           std::string,
                                           char,
                                           int,
                                           double>;

static_assert(std::variant_size<BasicTokenDataVariant>::value == (static_cast<int>(BasicTokenType::Last) + 1),
              "BasicTokenDataVariant must have same size as number of distinct values in BasicTokenType !");

//This class is just wrapper for BasicTokenDataVariant with convenience functions.
class BasicToken
{
public:
    //Makes Symbol token with empty text
    BasicToken() : BasicToken<BasicTokenType::Symbol>("") {};

    //DataT must be same or be convertible to type in BasicTokenDataVariant for supplied BasicTokenType
    template<BasicTokenType type, typename DataT>
    BasicToken(DataT&& arg) 
    {
        data.emplace<static_cast<int>(type)>(std::forward<DataT>(arg));
    }

    static BasicToken makeSymbol            (std::string data = {}){ return BasicToken<BasicTokenType::Symbol>(std::move(data)); }
    static BasicToken makeString            (std::string data = {}){ return BasicToken<BasicTokenType::String>(std::move(data)); }
    static BasicToken makeSpecialCharachter (char ch = 0){ return BasicToken<BasicTokenType::SpecialCharachter>(ch); }
    static BasicToken makeNumberInt         (int number = 0){ return BasicToken<BasicTokenType::NumberInt>(number); }
    static BasicToken makeNumberDouble      (double number = 0){ return BasicToken<BasicTokenType::NumberDouble>(number); }

    BasicTokenType type() const;

    bool isSymbol           () const;
    bool isString           () const;
    bool isSpecialCharachter() const;
    bool isNumberInt        () const;
    bool isNumberDouble     () const;
    bool isNumber           () const;

    std::string dataToString();
    

private:
    BasicTokenDataVariant data;
};