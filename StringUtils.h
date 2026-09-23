#pragma once
#include <string>
#include <optional>
#include <variant>
#include <cassert>

//Assumes base 10, expects strictly valid number strings, no trailing spacebars or anything - otherwise false is returned.
bool stringIsValidIntOrDouble(const std::string& text, bool& isInt);

std::optional<std::variant<int, double>> convertStringToIntOrDouble(const std::string& text);