#include "StringUtils.h"

bool stringIsValidIntOrDouble(const std::string& text, bool& isInt)
{
    int numPoints = 0;

    for (int i = 0; i < text.size(); ++i)
    {
        const auto ch       = text[i];
        
        const auto isDigit  = std::isdigit(ch);
        const auto isMinus  = ch == '-';
        const auto isPlus   = ch == '+';
        const auto isPoint  = ch == '.';

        const auto isFirstChar = i == 0;
        const auto isLastChar = i == text.size()-1;

        if (isPoint) numPoints++;

        const auto isValid = isDigit ||
                            (isMinus && isFirstChar) ||
                            (isPlus  && isFirstChar) ||
                            (isPoint && (!isFirstChar && !isLastChar && numPoints == 1));
        if(!isValid) return false;
    }

    isInt = numPoints == 0;
    return true;
}

std::optional<std::variant<int, double>> convertStringToIntOrDouble(const std::string& text)
{
    bool isInt;
    if (stringIsValidIntOrDouble(text, isInt))
    {
        //I assume that now conversion should always work without error.
        //I cant just use strtod directly - because i dont want fucking "NaN" string to be parsed as
        //"valid" double containing, you guessed it, NaN. Etc. There are many things like that in strtod implementation.

        char* endptr = nullptr;
        if (isInt)
        {
            auto value        = std::strtol(text.c_str(), &endptr, 10);
            bool fuckingError = endptr == text.c_str();
            assert(!fuckingError);
            return int(value);
        }
        else
        {
            auto value        = std::strtod(text.c_str(), &endptr);
            bool fuckingError = endptr == text.c_str();
            assert(!fuckingError);
            return value;
        }
    }
    else return {};
}