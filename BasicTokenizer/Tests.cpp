#include "Tests.h"
#include "BasicTokenizer.h"

bool tokenListsEqual(const BasicTokenList& expected, const BasicTokenList& actual)
{
    if(expected.size() != actual.size()) return false;

    auto itExpected = expected.begin();
    auto itActual   = actual.begin();

    while(itExpected != expected.end() && itActual != actual.end())
    {
        if (*itExpected != *itActual) return false;

        itExpected++;
        itActual++;
    }

    return true;
}

std::string tokenListToString(const BasicTokenList& list)
{
    std::string res = std::format("[{}]BasicTokenList{{ ", list.size());

    bool first = true;
    for (const auto& token : list)
    {
        if (first) first = false;
        else res += ", ";

        res += token.info();
    }
    res += " }}";
    return res;
}

std::string listOrErrorToString(const BasicTokenListOrError &listOrErr)
{
    if (listOrErr.index() == 0) return tokenListToString(std::get<BasicTokenList>(listOrErr));
    else return std::get<std::string>(listOrErr);
}

bool compareTokenListsAndPrintMismatch(const BasicTokenList& expected, const BasicTokenList& actual)
{
    if (!tokenListsEqual(expected, actual))
    {
        SV_ERROR(std::format("Token list mismatch. Expected and actual:\n{}\n{}",
                             tokenListToString(expected), tokenListToString(actual)));
        return false;
    }
    else return true;
}

bool compareResultsAndPrintMismatch(const BasicTokenListOrError &expected, const BasicTokenListOrError &actual)
{
    const bool expectedIsErr = expected.index() == 1;
    const bool actualIsErr = actual.index() == 1;

    if (expectedIsErr && actualIsErr)
    {
        return true; //not checking actual error equality
    }
    else if (!expectedIsErr && !actualIsErr)
    {
        return compareTokenListsAndPrintMismatch(std::get<BasicTokenList>(expected), std::get<BasicTokenList>(actual));
    }
    else
    {
        SV_ERROR(std::format("Result mismatch. Expected and actual:\n{}\n{}",
                             listOrErrorToString(expected), listOrErrorToString(actual)));
        return false;
    }
}

BasicToken sym(std::string str)
{
    return BasicToken::makeSymbol(std::move(str));
}
BasicToken str(std::string str)
{
    return BasicToken::makeString(std::move(str));
}
BasicToken spec(char ch)
{
    return BasicToken::makeSpecialCharachter(ch);
}

bool testDefaultConstructedTokenizer()
{
    BasicTokenizer tokenizer;

    std::string text = " \" f;e\nwf'\" -- , ";

    if( !compareResultsAndPrintMismatch(BasicTokenList{sym(text)},
                                        tokenizer.parse(text)) )
    {
        return false;
    }

    return true;
}



bool testBreakingChars()
{
    BasicTokenizer tokenizer;
    tokenizer.enableBreakCharachters(" \n\t");

    std::string text = "all work and\nno play\t";

    if( !compareResultsAndPrintMismatch(BasicTokenList{sym("all"), sym("work"), sym("and"), sym("no"), sym("play")},
                                        tokenizer.parse(text)) )
    {
        return false;
    }

    return true;
}

bool testStrings()
{
    BasicTokenizer tokenizer;
    tokenizer.enableBreakCharachters();
    tokenizer.enableParsingStringTokens();
    tokenizer.enableEscapingStringCharachters();

    std::string text = R"(all "work and \"no play\" makes" jack)";

    if( !compareResultsAndPrintMismatch(BasicTokenList{sym("all"), str(R"(work and "no play" makes)"), sym("jack")},
                                        tokenizer.parse(text)) )
    {
        return false;
    }

    return true;
}

bool testStringsAndSpecChars()
{
    BasicTokenizer tokenizer;
    tokenizer.enableBreakCharachters();
    tokenizer.enableParsingStringTokens();
    tokenizer.enableEscapingStringCharachters();
    tokenizer.enableParsingSpecialCharachterTokens(",+");

    std::string text = R"(all, all "work, and + \"no play\" makes" jack+dull+boy)";

    if( !compareResultsAndPrintMismatch(BasicTokenList{sym("all"), spec(','), sym("all"), str(R"(work, and + "no play" makes)"),
                                                        sym("jack"),spec('+'),sym("dull"),spec('+'),sym("boy")},
                                        tokenizer.parse(text)) )
    {
        return false;
    }

    return true;
}

void runBasicTokenizerTests()
{
    int totalTests = 0;
    int testsSuccessful = 0;

    #define SV_RUN_BASIC_TOKENIZER_TEST(FUNC) totalTests++; if(FUNC())\
                                                            {testsSuccessful++;}else\
                                                            {SV_ERROR(std::format("BasicTokenizer test {} has failed.", #FUNC));}

    SV_RUN_BASIC_TOKENIZER_TEST(testDefaultConstructedTokenizer);
    SV_RUN_BASIC_TOKENIZER_TEST(testBreakingChars);
    SV_RUN_BASIC_TOKENIZER_TEST(testStrings);
    SV_RUN_BASIC_TOKENIZER_TEST(testStringsAndSpecChars);

    auto resultInfo = std::format("BasicTokenizerTests: {} / {} successful.", testsSuccessful, totalTests);
    if (testsSuccessful == totalTests)
    {
        SV_LOG(resultInfo);
    }
    else SV_ERROR(resultInfo);
}