#pragma once

#include <string>

// IRC name normalization
std::string ToLower(const std::string& str);

// Identity validation / sanitization
bool IsValidNickname(const std::string& str);
std::string ToValidUsername(const std::string& str);
