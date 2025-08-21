#pragma once
#include <string>
#include <cryptopp/secblock.h>

using namespace CryptoPP;

namespace base {
	extern std::string enHex(const SecByteBlock &block);
}
