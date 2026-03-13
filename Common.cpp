#include "Common.h"

std::string getCurrentTimeHMS() {
    auto now = std::chrono::current_zone()->to_local(
        floor<std::chrono::seconds>(std::chrono::system_clock::now()));
    return std::format("{:%H:%M:%S}", now);
}