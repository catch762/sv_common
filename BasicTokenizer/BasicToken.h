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

const char* basicTokenTypeToString(BasicTokenType type);

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
    BasicToken();

    static BasicToken makeSymbol            (std::string data = {});
    static BasicToken makeString            (std::string data = {});
    static BasicToken makeSpecialCharachter (char ch = 0);
    static BasicToken makeNumberInt         (int number = 0);
    static BasicToken makeNumberDouble      (double number = 0);

    BasicTokenType type() const;

    bool isSymbol           () const;
    bool isString           () const;
    bool isSpecialCharachter() const;
    bool isNumberInt        () const;
    bool isNumberDouble     () const;
    bool isNumber           () const;

    //If you call it on a wrong type, it will LOG_ERROR and return default value.
    const std::string&  getSymbolData() const; 
    const std::string&  getStringData() const; 
    char                getSpecialCharachterData() const; 
    int                 getNumberIntData() const; 
    double              getNumberDoubleData() const;

    //If its NumberDouble its fine, if NumberInt, int data gets casted to double.
    //Anything else is error which will be logged and default value returned.
    double              getNumberDataAsDouble() const;

    //If you call it on a different type, it will change it.
    void setSymbolData(std::string data); 
    void setStringData(std::string data); 
    void setSpecialCharachterData(char ch); 
    void setNumberIntData(int num); 
    void setNumberDoubleData(double num); 

    //no matter whats inside, its converted to string and returned
    std::string dataToString() const;

    std::string info() const;

    bool operator==(const BasicToken& other) const;

private:
    //DataT must be same or be convertible to type in BasicTokenDataVariant for supplied BasicTokenType
    template<BasicTokenType type, typename DataT>
    void setTypeAndData(DataT&& arg) 
    {
        data.emplace<static_cast<int>(type)>(std::forward<DataT>(arg));
    }

private:
    BasicTokenDataVariant data;
};

SV_DECL_STD_FORMATTER(BasicToken, obj.info());

namespace basictoken_helpers //super short aliases, mostly to tidy up tests
{
BasicToken sym(std::string str);
BasicToken str(std::string str);
BasicToken spec(char ch);
BasicToken mkint(int num);
BasicToken mkdouble(double num);
};