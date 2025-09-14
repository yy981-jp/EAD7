#pragma once
#include <vector>
#include <string>
#include "../GUI/ui.h"

extern std::vector<std::string> ca;
extern void UI();
extern std::string inp(const std::string& out);
extern char choice(const std::string& message, const std::string& validChars);
