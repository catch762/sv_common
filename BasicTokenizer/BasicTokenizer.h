#pragma once
#include "BasicToken.h"
#include <list>

using BasicTokenList = std::list<BasicToken>;
SV_DECL_ERR(BasicTokenList);

class BasicTokenizer
{
public:
    //Enables parsing BasicTokenType::String tokens. You can supply multiple quote symbols.
    void enableParsingStringTokens(std::string _quoteCharsList = "\"");

    // Only works if string parsing is enabled.
    // Escaping chars is made via backslash (\), and it only works inside a string.
    // Supply pairs like {'n', '\n'}.
    // Dont forget to list quote chars, if you want them (list same char like {'\"', '\"'})
    void enableEscapingStringCharachters(std::map<char, char> _escapeCharsTranslationMap = {{'n',  '\n'},
                                                                                            {'t',  '\t'},
                                                                                            {'\"', '\"'}}); 

    //every encountered char from this list will be saved as BasicTokenType::SpecialCharachter
    void enableParsingSpecialCharachterTokens(std::string _specialCharachterList);
    
    // Things like space ' ' for example.
    // If such charachters is encountered, it stops parsing current token, saves it,
    // and starts parsing next one. The break charachter itself is not added as token.
    void enableBreakCharachters(std::string _breakCharachterList = " \t\n");

    void enableParsingNumbers();
    void enableApplyingMinusCharToNumberTokens();

public:
    // Returns either parsed tokens, or string containing error in case of failure
    BasicTokenListOrError parse(const std::string &text); 


private:
    using CharsParsedCount = int;
    SV_DECL_ERR(CharsParsedCount);

    CharsParsedCountOrError parseNextBlock(const std::string &text, int currentSymbolIdx);
    CharsParsedCount        parseNextBlockWhileParsingSymbol(const std::string &text, int currentSymbolIdx);
    CharsParsedCountOrError parseNextBlockWhileParsingString(const std::string &text, int currentSymbolIdx);

    //empty value returned means success
    StringErrOpt handleEndOfText();

    void startParsingSymbolToken(char firstChar);
    void addCharToSymbolTokenBeingParsed(char nextChar);
    void finishParsingSymbolToken();

    bool isStringQuoteChar(char ch);
    bool isBreakChar(char ch);
    bool isSpecialChar(char ch);
    bool isEscapeSlash(char ch);

    bool deleteConsequentMinusTokensInTheEndAndReturnIfCountWasOdd();

    void resetState();
//Settings that will probably not change once instance is set up:
private:
    std::string          quoteCharsList;            
    std::map<char, char> escapeCharsTranslationMap;
    std::string          specialCharachterList;
    std::string          breakCharachterList;
    bool                 parsingNumbersEnabled = false;
    bool                 applyingMinusTokensToNumbersEnabled = false;
//Parsing context that is reset on each parse operation:
private:
    enum class OngoingOperation
    {
        None,
        ParsingSymbol,
        ParsingString
    } ongoingOperation = OngoingOperation::None;

    std::string tokenBeingParsed;

    BasicTokenList tokens;
};