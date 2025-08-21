#include "base.h"
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>

namespace base {
	// バイト列を16進文字列に変換
	std::string enHex(const SecByteBlock &block) {
		std::string hex;
		for (size_t i = 0; i < block.size(); ++i) {
			char buf[3];
			sprintf(buf, "%02X", block[i]);
			hex += buf;
		}
		return hex;
	}
	
	std::string enHex(const std::vector<unsigned char>& data) {
		using namespace CryptoPP;
		std::string encoded;

		StringSource ss(data.data(), data.size(), true,
			new HexEncoder(
				new StringSink(encoded), true
			)
		);

		return encoded;
	}


	
	std::string en64(const std::vector<unsigned char>& data) {
		std::string encoded;

		StringSource ss(data.data(), data.size(), true,
			new Base64Encoder(
				new StringSink(encoded), false // falseで改行なし
			)
		);

		return encoded;
	}
}
