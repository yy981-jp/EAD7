#include <sodium.h>
#include <string>
#include <vector>
// #include <iostream>

#include "master.h"
#include "base.h"


// パラメータ（実運用で調整可能）
// libsodium 定義を使うのが簡単: MODERATE / SENSITIVE など
const unsigned long long PW_OPSLIMIT = crypto_pwhash_OPSLIMIT_MODERATE;
const size_t PW_MEMLIMIT = crypto_pwhash_MEMLIMIT_MODERATE;


MKEntryB64 WrapMK_WithPass(const BIN &mk,
						   const std::string &password)
{
	// salt 16B
	BIN salt(16);
	randombytes_buf(salt.data(), salt.size());

	// derive wrapKey (32B)
	BIN wrapKey(32);
	if (crypto_pwhash(wrapKey.data(), wrapKey.size(),
					  password.data(), password.size(),
					  salt.data(),
					  PW_OPSLIMIT, PW_MEMLIMIT,
					  crypto_pwhash_ALG_ARGON2ID13) != 0) {
		throw std::runtime_error("crypto_pwhash failed (memory/limits?)");
	}

	// nonce for XChaCha20-Poly1305: 24B
	BIN nonce(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
	randombytes_buf(nonce.data(), nonce.size());

	// allocate ciphertext buffer: mk.size() + ABYTES
	size_t ctlen = mk.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES;
	BIN ct(ctlen);

	unsigned long long outlen = 0;
	// optional AAD can be used (e.g. mkid or metadata). here none (nullptr,0)
	crypto_aead_xchacha20poly1305_ietf_encrypt(ct.data(), &outlen,
											   mk.data(), mk.size(),
											   nullptr, 0,
											   nullptr, // no nonce customization
											   nonce.data(), wrapKey.data());
	// ct already contains tag appended, outlen == ctlen
	(void)outlen;

	// clear wrapKey from memory
	delm(wrapKey);

	MKEntryB64 r;
	r.salt = base::enc64(salt);
	r.nonce = base::enc64(nonce);
	r.ct = base::enc64(ct);
	return r;
}

// unwrap: password + saved(salt,nonce,ct) -> derive wrapKey -> decrypt -> MK
BIN UnwrapMK_WithPass(const std::string &password,
											 const std::string &salt_b64,
											 const std::string &nonce_b64,
											 const std::string &ct_b64)
{
	auto salt = base::dec64(salt_b64);
	auto nonce = base::dec64(nonce_b64);
	auto ct = base::dec64(ct_b64);

	// derive wrapKey
	BIN wrapKey(32);
	if (crypto_pwhash(wrapKey.data(), wrapKey.size(),
					  password.data(), password.size(),
					  salt.data(),
					  PW_OPSLIMIT, PW_MEMLIMIT,
					  crypto_pwhash_ALG_ARGON2ID13) != 0) {
		throw std::runtime_error("crypto_pwhash failed");
	}

	// prepare output buffer: ctlen - ABYTES
	if (ct.size() < crypto_aead_xchacha20poly1305_ietf_ABYTES) throw std::runtime_error("ciphertext too short");
	size_t mlen = ct.size() - crypto_aead_xchacha20poly1305_ietf_ABYTES;
	BIN mk(mlen);
	unsigned long long mlen_out = 0;

	if (crypto_aead_xchacha20poly1305_ietf_decrypt(mk.data(), &mlen_out,
												   nullptr,
												   ct.data(), ct.size(),
												   nullptr, 0,
												   nonce.data(),
												   wrapKey.data()) != 0) {
		// auth failed
		delm(wrapKey);
		throw std::runtime_error("decryption failed (auth mismatch)");
	}

	// clear wrapKey
	delm(wrapKey);
	return mk;
}


BIN loadMKCore(const std::string& pass, const MKEntryB64& res) {
	// unwrap
	BIN mk = UnwrapMK_WithPass(pass, res.salt, res.nonce, res.ct);

	return mk;
}

MKEntryB64 createMKCore(const std::string& pass) {
	BIN mk = randomBIN(32);
/*
	std::cout << "Generated MK (raw hex): ";
	for (auto b : mk) printf("%02X", b);
	std::cout << "\n";
*/
	MKEntryB64 res = WrapMK_WithPass(mk, pass);

	// wipe mk from memory after wrap
	delm(mk);
	return res;
}
