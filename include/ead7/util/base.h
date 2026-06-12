#pragma once
#include <string>

#include <ead7/def/bin.h>
#include <ead7/def/def.h>


EAD7_BEGIN


namespace base {
	std::string encHex(const Bin &block);
	std::string enc64(const Bin& data);
	Bin dec64(const std::string& s);
}

namespace conv {
	inline std::string BintoSTR(const Bin& block) {
		return std::string(reinterpret_cast<const char*>(block.data()), block.size());
	}

	inline Bin STRtoBin(const std::string& str) {
		return Bin(reinterpret_cast<const byte*>(str.data()), str.size());
	}
	
	template <size_t N>
	inline Bin ARRtoBin(const std::array<uint8_t, N>& arr) {
		Bin bin(N);
		memcpy(bin.data(), arr.data(), N);
		return bin;
	}

	template <size_t N>
	inline std::array<uint8_t, N> BintoARR(const Bin& bin) {
		std::array<uint8_t, N> arr{};
		memcpy(arr.data(), bin.data(), std::min(N, bin.size()));
		return arr;
	}
}


EAD7_END
