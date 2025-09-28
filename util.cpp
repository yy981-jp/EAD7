#include "def.h"
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>
#include <cryptopp/hkdf.h>
#include <sodium.h>
#include <fstream>
#include <string>
#include <boost/locale.hpp>

#include "AES256GCM.h"

bool AESNI = true;


std::wstring to_wstring(const std::string& u8) {
	return boost::locale::conv::to_utf<wchar_t>(u8, "UTF-8");
}

json readJson(const std::string& path) {
	std::ifstream ifs(fs::path(to_wstring(path)));
	if (!ifs) throw std::runtime_error("readJson()::ファイルを開けませんでした");
	json j;
	ifs >> j;
	return j;
}

void writeJson(const std::string& path, const json& j) {
	std::ofstream ofs(fs::path(to_wstring(path)));
	if (!ofs) throw std::runtime_error("writeJson()::ファイルを開けませんでした");
	ofs << j;
}

CryptoGCM encAES256GCM(const BIN& key, const BIN& nonce, const BIN& text, const BIN& AAD) {
	if (AESNI) return encAES256GCM_sodium(key, nonce, text, AAD);		// AES-NI在り 高速
	else		return encAES256GCM_cryptopp(key, nonce, text, AAD);	// AES-NI無し 低速
}

BIN decAES256GCM(const BIN& key, const BIN& nonce, const BIN& text, const BIN& AAD, const BIN& tag) {
	if (AESNI) return decAES256GCM_sodium(key, nonce, text, AAD, tag);		// AES-NI在り 高速
	else		return decAES256GCM_cryptopp(key, nonce, text, AAD, tag);	// AES-NI無し 低速
}

BIN deriveKey(const BIN& ikm, const std::string &info, size_t keyLen, const BIN& salt) {
	BIN derived(keyLen);

	HKDF<SHA256> hkdf;
	hkdf.DeriveKey(derived, derived.size(), ikm, ikm.size(), salt, 0, 
				   reinterpret_cast<const byte*>(info.data()), info.size());

	return derived;
}

BIN randomBIN(size_t size) {
	BIN out(size);
	randombytes_buf(out.data(), out.size());
	return out;
}