#pragma once
#include <string>

#include "def.h"


namespace base {
	extern std::string encHex(const BIN &block);
	extern std::string encHex(const BIN& data);
	extern std::string enc64(const BIN& data);
	extern BIN dec64(const std::string& s);
}
