#pragma once
#include <string>
#include <cryptopp/secblock.h>

using namespace CryptoPP;

namespace base {
	extern std::string enHex(const SecByteBlock &block);
	extern std::string enHex(const std::vector<unsigned char>& data);
	extern std::string en64(const std::vector<unsigned char>& data);
}
