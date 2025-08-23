#include "base.h"
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>

#include "def.h"

namespace base {
	std::string encHex(const BIN& data) {
		std::string encoded;

		StringSource ss(data.data(), data.size(), true,
			new HexEncoder(
				new StringSink(encoded), true
			)
		);

		return encoded;
	}


	
	std::string enc64(const BIN& data) {
		std::string encoded;

		StringSource ss(data.data(), data.size(), true,
			new Base64Encoder(
				new StringSink(encoded), false // falseで改行なし
			)
		);

		return encoded;
	}
	
	BIN dec64(const std::string& s) {
		std::string bin;
		StringSource ss(s, true,
			new Base64Decoder(new StringSink(bin))
		);
		return BIN(reinterpret_cast<const unsigned char*>(bin.data()), bin.size());
	}
}
