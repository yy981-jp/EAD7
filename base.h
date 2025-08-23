#pragma once
#include <string>
#include <cryptopp/secblock.h>

#include "def.h"


namespace base {
	extern std::string enHex(const BIN &block);
	extern std::string enHex(const std::vector<unsigned char>& data);
	extern std::string en64(const std::vector<unsigned char>& data);
}
