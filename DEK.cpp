#include "master.h"


BIN deriveDEC(const BIN& kek, const BIN& nonce) {
	std::string info = "EAD7|DEC|v1";
	return deriveKey(kek, info, 32, nonce);
}

CryptoGCM_nonce encCore(const BIN& kek, const BIN& plaintext, const BIN& aad) {
	BIN nonce = randomBIN(12);
	BIN decKey = deriveDEC(kek, nonce);

	CryptoGCM_nonce result;
	CryptoGCM out = encAES256GCM(decKey, nonce, plaintext, aad);

	result.cipher = out.cipher;
	result.tag = out.tag;
	result.nonce = nonce;
	delm(decKey);
	return result;
}

BIN decCore(const BIN& kek, const BIN& nonce, const BIN& cipher, const BIN& aad, const BIN& tag) {
	BIN decKey = deriveDEC(kek, nonce);
	BIN plain = decAES256GCM(decKey, nonce, cipher, aad, tag);

	delm(decKey);
	return plain;
}
