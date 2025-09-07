#pragma once
#include <string>

#include "def.h"


namespace base {
	extern std::string encHex(const BIN &block);
	extern std::string enc64(const BIN& data);
	extern BIN dec64(const std::string& s);
}

namespace conv {
	inline std::string BINtoSTR(const BIN& block) {
		return std::string(reinterpret_cast<const char*>(block.BytePtr()), block.size());
	}

	inline BIN STRtoBIN(const std::string& str) {
		return BIN(reinterpret_cast<const byte*>(str.data()), str.size());
	}
}
