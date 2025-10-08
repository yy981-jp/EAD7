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
		return std::string(reinterpret_cast<const char*>(block.data()), block.size());
	}

	inline BIN STRtoBIN(const std::string& str) {
		return BIN(reinterpret_cast<const byte*>(str.data()), str.size());
	}
	
	template <size_t N>
	inline BIN ARRtoBIN(const std::array<uint8_t, N>& arr) {
		BIN bin(N);
		memcpy(bin.data(), arr.data(), N);
		return bin;
	}

	template <size_t N>
	inline std::array<uint8_t, N> BINtoARR(const BIN& bin) {
		std::array<uint8_t, N> arr{};
		memcpy(arr.data(), bin.data(), std::min(N, bin.size()));
		return arr;
	}
}
