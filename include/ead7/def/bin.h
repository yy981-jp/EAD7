#pragma once

#include <ead7/def/secure.h>
#include <ead7/def/def.h>
#include <cryptopp/config.h>

#include <vector>
#include <cstdint>


EAD7_BEGIN


using byte = uint8_t;

class Bin {
	std::vector<byte> bytes;

public:
	Bin() = default;
	Bin(const byte* data, size_t size): bytes(data, data + size) {}
	Bin(size_t size) { resize(size); }
	
	~Bin() { delm(bytes); }

	operator const CryptoPP::byte*() const {
		return static_cast<const CryptoPP::byte*>(data());
	}
	operator CryptoPP::byte*() {
		return static_cast<CryptoPP::byte*>(data());
	}


	constexpr byte* data() { return bytes.data(); }
	constexpr const byte* data() const { return bytes.data(); }

	size_t size() { return bytes.size(); }
	size_t size() const { return bytes.size(); }

	void cleanNew(size_t size) {
		delm(bytes);
		resize(size);
	}

	void resize(size_t size) { bytes.resize(size); }
};


EAD7_END
