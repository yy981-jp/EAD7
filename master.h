#pragma once
#include <string>
#include <vector>
#include <sodium.h>

#include "def.h"


template <typename T>
concept HasDataAndSize = requires(T t) {
	{ t.data() } -> std::convertible_to<void*>;
	{ t.size() } -> std::convertible_to<std::size_t>;
};

template <HasDataAndSize Bin>
void delm(Bin& bin) {
	sodium_memzero(bin.data(), bin.size());
}

template <HasDataAndSize... Bins>
void delm(Bins&... bins) {
	(delm(bins), ...);
}


inline std::string getMkid(const std::string& KIDPath) {
	return std::to_string(std::stoi(KIDPath.substr(0,2)));
}


enum class Status : uint8_t {
	Active,
	Disabled,
	Revoked,
};

struct KIDEntry {
	std::string label, status, note, b64;
	int64_t created;

	KIDEntry(std::string label, std::string note, std::string status = "active"): label(label), note(note), status(status) {}
	operator bool() {
		return (status == "active");
	}
};

struct MKEntryB64 {
	std::string salt,nonce,ct;
	bool status = true;
	MKEntryB64() {}
	MKEntryB64(bool status): status(status) {}
	operator bool() {return status;}
};

struct CryptoGCM {BIN cipher, tag;};
struct CryptoGCM_nonce {BIN cipher, tag, nonce;};

// util
extern BIN deriveKey(const BIN& ikm, const std::string &info, size_t keyLen, const BIN& salt = BIN());
extern BIN randomBIN(size_t size);
extern CryptoGCM encAES256GCM(const BIN& key, const BIN& nonce, const BIN& text, const BIN& AAD);
extern BIN decAES256GCM(const BIN& key, const BIN& nonce, const BIN& text, const BIN& AAD, const BIN& tag);
extern json readJson(const std::string& path);
extern void writeJson(const std::string& path, const json& j);

// MK
extern MKEntryB64 createMKCore(const std::string& pass, BIN mk = randomBIN(32));
extern BIN loadMKCore(const std::string& pass, const MKEntryB64& res);
extern void createMK(int index, const std::string& pass, BIN mk = randomBIN(32));
extern BIN loadMK(int index, const std::string& pass);

// KID
extern void saveKID(const BIN& mk, const int &mkid, const ordered_json& body);
extern ordered_json loadKID(const BIN& mk, const int& mkid);
extern void addNewKid(ordered_json& body, const KIDEntry& kid_e);

// KEK
extern json createRawKEK(const BIN& mk, json kek_json, const json& kid_json);
extern json createAdmKEK(const BIN& mk, const json& raw_json);
extern json decAdmKEK(const BIN& mk, const json& adm_json);
extern json createPKEK(const json& raw_json);
extern json decPKEK(const json& p_json);
extern json createDstKEK(const std::string &password, unsigned long long opslimit, size_t memlimit);
extern json decDstKEK(const std::string &password, const json &dst_json);

// Core (DEK.cpp)
extern CryptoGCM_nonce encCore(const BIN& kek, const BIN& plaintext, const BIN& aad);
extern BIN decCore(const BIN& kek, const BIN& nonce, const BIN& cipher, const BIN& aad, const BIN& tag);

// token
extern void saveToken(const BIN& token);
extern BIN loadToken();

// master view
extern void mmain();
