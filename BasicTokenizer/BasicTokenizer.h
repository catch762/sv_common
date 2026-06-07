#pragma once
#include "BasicToken.h"
#include <list>

using BasicTokenList = std::list<BasicToken>;

class BasicTokenizer
{
public:
    void enableParsingStringTokens(std::string _quoteCharsList = "\"");

    // Only works if string parsing is enabled.
    // Escaping chars is made via backslash (\), and it only works inside a string.
    // Supply pairs like {'n', '\n'}.
    // Dont forget to list quote chars, if you want them (list same char like {'\"', '\"'})
    void enableEscapingStringCharachters(std::map<char, char> _escapeCharsTranslationMap = {{'n',  '\n'},
                                                                                            {'t',  '\t'},
                                                                                            {'\"', '\"'}}); 

    void enableParsingSpecialCharachterTokens(std::string _specialCharachterList);
    
    // If such charachters is encountered, it stops parsing current token, saves it,
    // and starts parsing next one. The break charachter itself is not added as token.
    void setBreakCharachters(std::string _breakCharachterList = " \t\n");

public:    
    BasicTokenList parse(const std::string &text); 


private:
    

//Settings that will not change once instance is set up:
private:
    std::string          quoteCharsList;            
    std::map<char, char> escapeCharsTranslationMap;
    std::string          specialCharachterList;
    std::string          breakCharachterList;

//Parsing context that is reset on each parse operation:
private:
    
};