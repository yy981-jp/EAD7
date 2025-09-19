#pragma once
#include <string>
#include <vector>
#include <sodium.h>

#include "def.h"
#include "del.h"


namespace HEADER {
	constexpr uint8_t magic(1), ver(1), mkid(1), kid(16), nonce(12), tag(16),
		magicData(0xE7), verData(1),
		all(magic+ver+mkid+kid+nonce+tag);
}

inline std::string getMkid(const std::string& KIDPath) {
	return std::to_string(std::stoi(KIDPath.substr(0,2)));
}
/*
enum class FSType {
	MK, kid, p_kek, raw_kek, cus_kek, adm_kek, dst_kek,
	fe_e7
}
*/
enum class Status {
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
extern void saveKID(const BIN& mk, const uint8_t& mkid, const ordered_json& body);
extern json loadKID(const BIN& mk, const uint8_t& mkid);
extern void addNewKid(ordered_json& body, const KIDEntry& kid_e);

// KEK
extern json createRawKEK(const BIN& mk, json kek_json, const json& kid_json, const uint8_t& mkid);
extern json encAdmKEK(const BIN& mk, const json& raw_json, const uint8_t& mkid);
extern json decAdmKEK(const BIN& mk, const json& adm_json);
extern json encPKEK(const json& raw_json);
extern json decPKEK(const json& p_json);
extern json encDstKEK(const std::string &password, const json &raw_json, unsigned long long opslimit = crypto_pwhash_OPSLIMIT_INTERACTIVE, size_t memlimit = crypto_pwhash_MEMLIMIT_INTERACTIVE);
extern json decDstKEK(const std::string &password, const json &dst_json);

// Core (DEK.cpp)
extern CryptoGCM_nonce encCore(const BIN& kek, const BIN& plaintext, const BIN& aad);
extern BIN decCore(const BIN& kek, const BIN& nonce, const BIN& cipher, const BIN& aad, const BIN& tag);
extern BIN enc(const BIN& kek, const BIN& plaintext, const BIN& aad, const uint8_t& mkid, const BIN& kid);
extern bool dec(const BIN& kek, const BIN& blob, const BIN& aad, BIN& out_plain);

// token
extern void saveToken(const BIN& token);
extern BIN loadToken();

// master view
extern void mmain();
