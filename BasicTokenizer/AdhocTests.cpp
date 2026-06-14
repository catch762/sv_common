#include "AdhocTests.h"
#include "BasicTokenizer.h"

using namespace basictoken_helpers;



bool tokenListsEqual(const BasicTokenVec& expected, const BasicTokenVec& actual)
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

std::string tokenListToString(const BasicTokenVec& list)
{
    std::string res = std::format("[{}]BasicTokenVec{{ ", list.size());

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

std::string listOrErrorToString(const BasicTokenVecOrError &listOrErr)
{
    if (listOrErr.index() == 0) return tokenListToString(std::get<BasicTokenVec>(listOrErr));
    else return std::get<std::string>(listOrErr);
}

bool compareTokenListsAndPrintMismatch(const BasicTokenVec& expected, const BasicTokenVec& actual)
{
    if (!tokenListsEqual(expected, actual))
    {
        SV_ERROR(std::format("Token list mismatch. Expected and actual:\n{}\n{}",
                             tokenListToString(expected), tokenListToString(actual)));
        return false;
    }
    else return true;
}

bool compareResultsAndPrintMismatch(const BasicTokenVecOrError &expected, const BasicTokenVecOrError &actual)
{
    const bool expectedIsErr = expected.index() == 1;
    const bool actualIsErr = actual.index() == 1;

    if (expectedIsErr && actualIsErr)
    {
        return true; //not checking actual error equality
    }
    else if (!expectedIsErr && !actualIsErr)
    {
        return compareTokenListsAndPrintMismatch(std::get<BasicTokenVec>(expected), std::get<BasicTokenVec>(actual));
    }
    else
    {
        SV_ERROR(std::format("Result mismatch. Expected and actual:\n{}\n{}",
                             listOrErrorToString(expected), listOrErrorToString(actual)));
        return false;
    }
}


bool testDefaultConstructedTokenizer()
{
    BasicTokenizer tokenizer;

    std::string text = " \" f;e\nwf'\" -- , ";

    return compareResultsAndPrintMismatch(BasicTokenVec{sym(text)},
                                        tokenizer.parse(text));
}



bool testBreakingChars()
{
    BasicTokenizer tokenizer;
    tokenizer.enableBreakCharachters(" \n\t");

    std::string text = "all work and\nno play\t";

    return compareResultsAndPrintMismatch(BasicTokenVec{sym("all"), sym("work"), sym("and"), sym("no"), sym("play")},
                                        tokenizer.parse(text));
}

bool testStrings()
{
    BasicTokenizer tokenizer;
    tokenizer.enableBreakCharachters();
    tokenizer.enableParsingStringTokens();
    tokenizer.enableEscapingStringCharachters();

    std::string text = R"(all "work and \"no play\" makes" jack)";

    return compareResultsAndPrintMismatch(BasicTokenVec{sym("all"), str(R"(work and "no play" makes)"), sym("jack")},
                                        tokenizer.parse(text));
}

bool testStringsAndSpecChars()
{
    BasicTokenizer tokenizer;
    tokenizer.enableBreakCharachters();
    tokenizer.enableParsingStringTokens();
    tokenizer.enableEscapingStringCharachters();
    tokenizer.enableParsingSpecialCharachterTokens(",+");

    std::string text = R"(all, all "work, and + \"no play\" makes" jack+dull+boy)";

    return compareResultsAndPrintMismatch(BasicTokenVec{sym("all"), spec(','), sym("all"), str(R"(work, and + "no play" makes)"),
                                                        sym("jack"),spec('+'),sym("dull"),spec('+'),sym("boy")},
                                        tokenizer.parse(text));
}

bool testNumbersParsing()
{
    BasicTokenizer tokenizer;
    tokenizer.enableBreakCharachters();
    tokenizer.enableParsingStringTokens("'\"");
    tokenizer.enableEscapingStringCharachters();
    tokenizer.enableParsingSpecialCharachterTokens(",+-.");
    tokenizer.enableParsingNumbers();
    tokenizer.enableApplyingMinusCharToNumberTokens();

    std::string text = R"(
            hello 0 -1 10.0 ---5.15 'hi -5.0'
            aaa.5 5.bbb
    )";
    BasicTokenVec expected = {
        sym("hello"), mkint(0), mkint(-1), mkdouble(10.0), mkdouble(-5.15), str("hi -5.0"),
        sym("aaa"), spec('.'), mkint(5), sym("5.bbb")
    };

    return compareResultsAndPrintMismatch(expected, tokenizer.parse(text));
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
    SV_RUN_BASIC_TOKENIZER_TEST(testNumbersParsing);

    auto resultInfo = std::format("BasicTokenizerTests: {} / {} successful.", testsSuccessful, totalTests);
    if (testsSuccessful == totalTests)
    {
        SV_LOG(resultInfo);
    }
    else SV_ERROR(resultInfo);
}