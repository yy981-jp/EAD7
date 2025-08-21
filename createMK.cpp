// libsodium_wrap.cpp
// build: g++ -std=c++23 libsodium_wrap.cpp -lsodium -o libsodium_wrap
#include <sodium.h>
#include <string>
#include <vector>
#include <iostream>
	#include <yy981/type.h>

// helpers: Base64 encode/decode using libsodium
std::string bin2base64(const unsigned char* bin, size_t binlen) {
	std::string out;
	size_t olen = sodium_base64_ENCODED_LEN(binlen, sodium_base64_VARIANT_URLSAFE_NO_PADDING);
	out.resize(olen);
	sodium_bin2base64(&out[0], olen, bin, binlen, sodium_base64_VARIANT_URLSAFE_NO_PADDING);
	// remove trailing nulls
	while (!out.empty() && out.back() == '\0') out.pop_back();
	return out;
}
std::vector<unsigned char> base642bin(const std::string &b64) {
	std::vector<unsigned char> out(b64.size()); // upper bound
	size_t binlen = 0;
	if (sodium_base642bin(out.data(), out.size(),
						  b64.c_str(), b64.size(),
						  nullptr, &binlen, nullptr,
						  sodium_base64_VARIANT_URLSAFE_NO_PADDING) != 0) {
		throw std::runtime_error("Base64 decode failed");
	}
	out.resize(binlen);
	return out;
}

// パラメータ（実運用で調整可能）
// libsodium 定義を使うのが簡単: MODERATE / SENSITIVE など
const unsigned long long PW_OPSLIMIT = crypto_pwhash_OPSLIMIT_MODERATE;
const size_t PW_MEMLIMIT = crypto_pwhash_MEMLIMIT_MODERATE;

// 生成: ランダム MK (32B)
std::vector<unsigned char> GenerateRandomMK() {
	std::vector<unsigned char> mk(32);
	randombytes_buf(mk.data(), mk.size());
	return mk;
}

// wrap: パスワード -> wrapKey (Argon2id) -> AEAD encrypt MK
// 戻り: salt_base64, nonce_base64, ct_base64
struct WrapResult {
	std::string salt_b64;
	std::string nonce_b64;
	std::string ct_b64;
};
WrapResult WrapMK_WithPass(const std::vector<unsigned char> &mk,
						   const std::string &password)
{
	// salt 16B
	std::vector<unsigned char> salt(16);
	randombytes_buf(salt.data(), salt.size());

	// derive wrapKey (32B)
	std::vector<unsigned char> wrapKey(32);
	if (crypto_pwhash(wrapKey.data(), wrapKey.size(),
					  password.data(), password.size(),
					  salt.data(),
					  PW_OPSLIMIT, PW_MEMLIMIT,
					  crypto_pwhash_ALG_ARGON2ID13) != 0) {
		throw std::runtime_error("crypto_pwhash failed (memory/limits?)");
	}

	// nonce for XChaCha20-Poly1305: 24B
	std::vector<unsigned char> nonce(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
	randombytes_buf(nonce.data(), nonce.size());

	// allocate ciphertext buffer: mk.size() + ABYTES
	size_t ctlen = mk.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES;
	std::vector<unsigned char> ct(ctlen);

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
	sodium_memzero(wrapKey.data(), wrapKey.size());

	WrapResult r;
	r.salt_b64 = bin2base64(salt.data(), salt.size());
	r.nonce_b64 = bin2base64(nonce.data(), nonce.size());
	r.ct_b64 = bin2base64(ct.data(), ct.size());
	return r;
}

// unwrap: password + saved(salt,nonce,ct) -> derive wrapKey -> decrypt -> MK
std::vector<unsigned char> UnwrapMK_WithPass(const std::string &password,
											 const std::string &salt_b64,
											 const std::string &nonce_b64,
											 const std::string &ct_b64)
{
	auto salt = base642bin(salt_b64);
	auto nonce = base642bin(nonce_b64);
	auto ct = base642bin(ct_b64);

	// derive wrapKey
	std::vector<unsigned char> wrapKey(32);
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
	std::vector<unsigned char> mk(mlen);
	unsigned long long mlen_out = 0;

	if (crypto_aead_xchacha20poly1305_ietf_decrypt(mk.data(), &mlen_out,
												   nullptr,
												   ct.data(), ct.size(),
												   nullptr, 0,
												   nonce.data(),
												   wrapKey.data()) != 0) {
		// auth failed
		sodium_memzero(wrapKey.data(), wrapKey.size());
		throw std::runtime_error("decryption failed (auth mismatch)");
	}

	// clear wrapKey
	sodium_memzero(wrapKey.data(), wrapKey.size());
	return mk;
}

// デモ
int main() {
	if (sodium_init() < 0) {
		std::cerr << "libsodium init failed\n";
		return 1;
	}

	std::string pass = "correct horse battery staple"; // 実運用ではpromptで入力
	auto mk = GenerateRandomMK();
	std::cout << "MK___:" << getType(mk) << "\n";

	std::cout << "Generated MK (raw hex): ";
	for (auto b : mk) printf("%02X", b);
	std::cout << "\n";

	auto res = WrapMK_WithPass(mk, pass);
	std::cout << "salt_b64: " << res.salt_b64 << "\n";
	std::cout << "nonce_b64: " << res.nonce_b64 << "\n";
	std::cout << "ct_b64: " << res.ct_b64 << "\n";

	// wipe mk from memory after wrap
	sodium_memzero(mk.data(), mk.size());

	// unwrap
	auto mk2 = UnwrapMK_WithPass(pass, res.salt_b64, res.nonce_b64, res.ct_b64);
	std::cout << "Unwrapped MK (hex): ";
	for (auto b : mk2) printf("%02X", b);
	std::cout << "\n";

	// clear mk2
	sodium_memzero(mk2.data(), mk2.size());
	return 0;
}
