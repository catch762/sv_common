#include "BasicTokenizer.h"

void BasicTokenizer::enableParsingStringTokens(std::string _quoteCharsList)
{
    SV_ASSERT(!_quoteCharsList.empty() && "Why do you enableParsingStringTokens() but dont supply a single quote char?");
    quoteCharsList = std::move(_quoteCharsList);
}

void BasicTokenizer::enableEscapingStringCharachters(std::map<char, char> _escapeCharsTranslationMap)
{
    SV_ASSERT(!quoteCharsList.empty() && "call enableParsingStringTokens() first !");
    escapeCharsTranslationMap = std::move(_escapeCharsTranslationMap);
}

void BasicTokenizer::enableParsingSpecialCharachterTokens(std::string _specialCharachterList)
{
    specialCharachterList = std::move(_specialCharachterList);
}

void BasicTokenizer::enableBreakCharachters(std::string _breakCharachterList)
{
    breakCharachterList = std::move(_breakCharachterList);
}

void BasicTokenizer::enableParsingNumbers()
{
    parsingNumbersEnabled = true;
}

void BasicTokenizer::enableApplyingMinusCharToNumberTokens()
{
    SV_ASSERT(specialCharachterList.find('-') != std::string::npos &&
        "If you want applying minus char tokens to numbers, you must add '-' char to "
        "enableParsingSpecialCharachterTokens() so that there are minus tokens in the first place");

    applyingMinusTokensToNumbersEnabled = true;
}

BasicTokenListOrError BasicTokenizer::parse(const std::string &text)
{
    if(text.empty())
    {
        resetState();
        return "BasicTokenizer: tried to parse empty text";
    }

    int currentSymbolIdx = 0;
    while(currentSymbolIdx < text.size())
    {
        CharsParsedCountOrError result = parseNextBlock(text, currentSymbolIdx);
        bool ok = std::holds_alternative<int>(result);

        if (ok)
        {
            int charsParsed = std::get<int>(result);
            currentSymbolIdx += charsParsed;
        }
        else
        {
            resetState();
            return std::format("During parsing text block on char [{}], encountered error: {}", currentSymbolIdx, std::get<std::string>(result));
        }
    }

    auto handleEndError = handleEndOfText();
    if (handleEndError)
    {
        resetState();
        return std::format("At the end of parsed text, encountered error: {}", *handleEndError);
    }

    BasicTokenList tokensToReturn = std::move(tokens);
    resetState();
    return std::move(tokensToReturn);
}

BasicTokenizer::CharsParsedCountOrError BasicTokenizer::parseNextBlock(const std::string &text, int currentSymbolIdx)
{
    if (currentSymbolIdx >= text.size() || currentSymbolIdx < 0)
    {
        return std::format("Error trying to get charachter [{}] from text with size [{}].", currentSymbolIdx, text.size());
    }

    if (ongoingOperation == OngoingOperation::ParsingSymbol)
    {
        return parseNextBlockWhileParsingSymbol(text, currentSymbolIdx);
    }
    else if (ongoingOperation == OngoingOperation::ParsingString)
    {
        return parseNextBlockWhileParsingString(text, currentSymbolIdx);
    }
    else SV_ASSERT(ongoingOperation == OngoingOperation::None);

    const char ch = text[currentSymbolIdx];

    if (isBreakChar(ch))
    {
        //keep going without saving break char itself
        return 1;
    }
    else if (isSpecialChar(ch))
    {
        tokens.push_back( BasicToken::makeSpecialCharachter(ch) );
        return 1;
    }
    else if (isStringQuoteChar(ch))
    {
        //also not saving quote char itself anywhere
        ongoingOperation = OngoingOperation::ParsingString;
        return 1;
    }
    else
    {
        //None of the above categories means we are starting to parse a symbol.
        startParsingSymbolToken(ch);
        return 1;
    }
}

BasicTokenizer::CharsParsedCount BasicTokenizer::parseNextBlockWhileParsingSymbol(const std::string &text, int currentSymbolIdx)
{
    SV_ASSERT(currentSymbolIdx < text.size());
    const char ch = text[currentSymbolIdx];

    auto shouldTreatThisSpecialCharAsNormal = [this](char ch)
    {
        // The only such condition currently is: if "." is special char, we dont break Symbol token
        // parsing if there are only digits before it. So its likely a number (its not a given yet)
        // Potential issue is 123.ZZZ will be parsed as single Symbol token, so '.' special status
        // will be ignored even though we dont even make a Number token in the end
        if (parsingNumbersEnabled && ch == '.')
        {
            bool allDigits = std::all_of(tokenBeingParsed.begin(), tokenBeingParsed.end(),
                                         [](char c) { return std::isdigit(c); });
            return allDigits;
        }
        else return false;
    };

    if (isBreakChar(ch))
    {
        finishParsingSymbolToken();
        return 1;
    }
    else if (isSpecialChar(ch) && !shouldTreatThisSpecialCharAsNormal(ch))
    {
        finishParsingSymbolToken();
        tokens.push_back( BasicToken::makeSpecialCharachter(ch) );
        return 1;
    }
    else if (isStringQuoteChar(ch))
    {
        finishParsingSymbolToken();
        ongoingOperation = OngoingOperation::ParsingString;
        return 1;
    }
    else
    {
        addCharToSymbolTokenBeingParsed(ch);
        return 1;
    }
}

BasicTokenizer::CharsParsedCountOrError BasicTokenizer::parseNextBlockWhileParsingString(const std::string &text, int currentSymbolIdx)
{
    SV_ASSERT(currentSymbolIdx < text.size());
    const char ch = text[currentSymbolIdx];

    if (isStringQuoteChar(ch))
    {
        //Found closing quote, string is finished.

        tokens.push_back( BasicToken::makeString(std::move(tokenBeingParsed)) );

        tokenBeingParsed.clear();
        ongoingOperation = OngoingOperation::None;

        return 1;
    }
    else if (isEscapeSlash(ch))
    {
        int nextCharIdx = currentSymbolIdx + 1;
        if (nextCharIdx >= text.size())
        {
            return std::format("Parse error, string ended with escape slash and nothing afterwards",
                               nextCharIdx, text.size());
        }

        char nextCh = text[nextCharIdx]; //actual char we r escaping
        auto translatedChar = getValueOpt(escapeCharsTranslationMap, nextCh);
        if (!translatedChar)
        {
            return std::format("Error, found escaped char [{}] inside a string. But its not legal char for escaping.", nextCh);
        }

        // successfully found what escaped char is translated to, now just add it to string and keep going
        tokenBeingParsed += *translatedChar;
        // [!] we already handled this char(slash) and next char(escaped one), thats 2 chars total
        return 2; 
    }
    else
    {
        // anything else is fine, just keep adding it to string
        tokenBeingParsed += ch;
        return 1;
    }
}

StringErrOpt BasicTokenizer::handleEndOfText()
{
    if (ongoingOperation == OngoingOperation::ParsingString)
    {
        return "Unfinished string, didnt find closing quote before end of text";
    }
    else if (ongoingOperation == OngoingOperation::ParsingSymbol)
    {
        //its fine, we just stop here
        finishParsingSymbolToken();

        return {};
    }
    else
    {
        //also fine
        return {};
    }
}

void BasicTokenizer::startParsingSymbolToken(char firstChar)
{
    ongoingOperation = OngoingOperation::ParsingSymbol;
    tokenBeingParsed = firstChar;
}

void BasicTokenizer::addCharToSymbolTokenBeingParsed(char nextChar)
{
    tokenBeingParsed += nextChar;
}

void BasicTokenizer::finishParsingSymbolToken()
{
    SV_ASSERT(!tokenBeingParsed.empty());

    auto doParseAsRegularSymbol = [&]()
    {
        tokens.push_back( BasicToken::makeSymbol(std::move(tokenBeingParsed)) );
    };

    if (parsingNumbersEnabled)
    {
        if (auto intOrDouble = convertStringToIntOrDouble(tokenBeingParsed))
        {
            int negationMod = applyingMinusTokensToNumbersEnabled &&
                              deleteConsequentMinusTokensInTheEndAndReturnIfCountWasOdd() ?
                              -1 : 1;

            if(std::holds_alternative<int>(*intOrDouble))
            {
                tokens.push_back(BasicToken::makeNumberInt(std::get<int>(*intOrDouble) * negationMod));
            }
            else
            {
                tokens.push_back(BasicToken::makeNumberDouble(std::get<double>(*intOrDouble) * negationMod));
            }
        }
        else doParseAsRegularSymbol();
    }
    else doParseAsRegularSymbol();

    tokenBeingParsed.clear();
    ongoingOperation = OngoingOperation::None;
}

bool BasicTokenizer::isStringQuoteChar(char ch)
{
    return quoteCharsList.find(ch) != std::string::npos;
}

bool BasicTokenizer::isBreakChar(char ch)
{
    return breakCharachterList.find(ch) != std::string::npos;
}

bool BasicTokenizer::isSpecialChar(char ch)
{
    return specialCharachterList.find(ch) != std::string::npos;
}

bool BasicTokenizer::isEscapeSlash(char ch)
{
    return ch == '\\';
}

bool BasicTokenizer::deleteConsequentMinusTokensInTheEndAndReturnIfCountWasOdd()
{
    int consequentMinusesEncountered = 0;

    auto it = tokens.end();

    while(it != tokens.begin())
    {
        --it;

        bool isMinus = it->isSpecialCharachter() && it->getSpecialCharachterData() == '-';

        if (isMinus)
        {
            it = tokens.erase(it);
            consequentMinusesEncountered++;
        }
        else break;
    }

    return consequentMinusesEncountered % 2 != 0;
}

void BasicTokenizer::resetState()
{
    ongoingOperation = OngoingOperation::None;
    tokenBeingParsed.clear();
    tokens.clear();
}
