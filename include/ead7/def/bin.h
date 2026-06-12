#pragma once

#include <ead7/def/secure.h>
#include <ead7/def/def.h>

#include <vector>
#include <cstdint>


EAD7_BEGIN


using byte = uint8_t;

class Bin {
	std::vector<byte> bytes;

public:
	Bin(const byte* data, size_t size): bytes(data, data + size) {}
	
	~Bin() {
		delm(bytes);
	}


	constexpr byte* data() { return bytes.data(); }
	constexpr const byte* data() const { return bytes.data(); }

	size_t size() { return bytes.size(); }
	size_t const size() const { return bytes.size(); }
};


EAD7_END
