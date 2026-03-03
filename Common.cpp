#include "Common.h"

std::string getCurrentTimeHMS() {
    auto now = floor<std::chrono::seconds>(std::chrono::system_clock::now());
    return std::format("{:%H:%M:%S}", now);
}