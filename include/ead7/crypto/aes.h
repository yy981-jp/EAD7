#pragma once
#include <ead7/def/bin.h>
#include <ead7/def/def.h>

#include <sodium.h>


EAD7_BEGIN


struct CryptoGCM {
	Bin cipher, tag;
};


class AES256GCM {
	bool AES_NI;

	CryptoGCM enc_sodium(const Bin& key, const Bin& nonce, const Bin& plaintext, const Bin& aad);
	Bin dec_sodium(const Bin& key, const Bin& nonce, const Bin& ciphertext, const Bin& aad, const Bin& tag);
	CryptoGCM enc_cryptopp(const Bin& key, const Bin& nonce, const Bin& plaintext, const Bin& aad);
	Bin dec_cryptopp(const Bin& key, const Bin& nonce, const Bin& ciphertext, const Bin& aad, const Bin& tag);


public:
	AES256GCM(bool AES_NI): AES_NI(AES_NI) {}

	CryptoGCM enc(const Bin& key, const Bin& nonce, const Bin& plaintext, const Bin& aad) {
		if (AES_NI) return enc_sodium(key, nonce, plaintext, aad);
			else	return enc_cryptopp(key, nonce, plaintext, aad);
	}

	Bin decAES256GCM(const Bin& key, const Bin& nonce, const Bin& text, const Bin& tag, const Bin& AAD) {
		if (AES_NI) return dec_sodium(key, nonce, text, AAD, tag);
			else	return dec_cryptopp(key, nonce, text, AAD, tag);
	}

};


EAD7_END
