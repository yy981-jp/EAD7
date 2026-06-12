#include <ead7/def/def.h>
#include <ead7/util/base.h>

#include <cryptopp/base64.h>
#include <cryptopp/hex.h>
#include <string>


EAD7_BEGIN


namespace base {
	using namespace CryptoPP;

	std::string encHex(const Bin& data) {
		std::string encoded;

		StringSource ss(data.data(), data.size(), true,
			new HexEncoder(
				new StringSink(encoded), true
			)
		);

		return encoded;
	}


	
	std::string enc64(const Bin& data) {
		std::string encoded;

		StringSource ss(data.data(), data.size(), true,
			new Base64URLEncoder(
				new StringSink(encoded), false
			)
		);

		return encoded;
	}
	
	Bin dec64(const std::string& s) {
		std::string bin;
		StringSource ss(s, true,
			new Base64URLDecoder(new StringSink(bin))
		);
		return Bin(reinterpret_cast<const unsigned char*>(bin.data()), bin.size());
	}
}


EAD7_END
