#include "LimitsAndRanges.h"
#include <cmath>
#include <algorithm>

void normalizeRange(double& min, double& max)
{
    if (min > max) std::swap(min, max);
}
void normalizeRange(double& min, double& max, double& val)
{
    normalizeRange(min, max);
    val = std::clamp(val, min, max);
}

double value11To01(double value11u)
{
    auto value01u = (value11u + 1.0) * 0.5;
    return std::clamp(value01u, 0.0, 1.0);
}

double value01To11(double value01u)
{
    auto value11u = value01u * 2.0 - 1.0;
    return std::clamp(value11u, -1.0, 1.0);
}

//returns 0 if value is at left, returns 1 if value is at right. Result clamped to [0,1]
double getValue01Clamped(double value, double left, double right)
{
    double span = right - left;

    if (abs(span) < (std::numeric_limits<double>::epsilon() * 100.0))
    {
        return 0.0;
    }

    auto res = std::clamp((value - left) / span, 0.0, 1.0);

    return res;
}

double getValue11Clamped(double value, double left, double right)
{
    return value01To11(getValue01Clamped(value, left, right));
}