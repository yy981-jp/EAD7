#pragma once
#include <string>
#include <vector>
#include <sodium.h>

#include "def.h"


#define delm(bin) sodium_memzero((bin).data(), (bin).size())

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

struct CryptoGCM {
	BIN cipher, tag;
};

extern MKEntryB64 createMKCore(const std::string& pass);
extern BIN loadMKCore(const std::string& pass, const MKEntryB64& res);

extern void createMK(int index, const std::string& pass);
extern BIN loadMK(int index, const std::string& pass);

extern void saveKID(const BIN& mk, const int &mkid, const ordered_json& body);
extern ordered_json loadKID(const BIN& mk, const int& mkid);
extern void addNewKid(ordered_json& body, const KIDEntry& kid_e);

json createRawKEK(const BIN& mk, json kek_json, const json& kid_json);


extern BIN deriveKey(const BIN& ikm, const std::string &info, size_t keyLen, const BIN& salt = BIN());
extern CryptoGCM encAES256GCM(const BIN& key, const BIN& nonce, const BIN& text, const BIN& AAD);
extern BIN decAES256GCM(const BIN& key, const BIN& nonce, const BIN& text, const BIN& AAD);
extern BIN randomBIN(size_t size);

void mmain();

// std::vector<KIDList> parseKIDList(const std::string& fn, const BIN &hmacKey);
