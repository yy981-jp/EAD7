#pragma once
#include <string>
#include <vector>
#include <sodium.h>

#include "def.h"
#include "del.h"

namespace HEADER {
	constexpr uint8_t magic(1), ver(1), mkid(1), kid(16), nonce(12), tag(16),
		magicData(0xE7), verData(1), verDataF(1),
		aad_all(magic + ver + mkid + kid + nonce),
		all(magic + ver + mkid + kid + nonce + tag);
}

namespace FHEADER {
	constexpr uint8_t magic(8), ver(1), verData(1);
	constexpr uint64_t magicData = 0x31383979795F46E7; // "E7F_y981"
}

namespace CHUNKSIZE {
	constexpr uint64_t default_size = 1ULL * 1024 * 1024, // 1 MiB
		min = 4ULL * 1024,								  // 4 KiB
		max = 16ULL * 1024 * 1024;						  // 16 MiB (safety upper bound)
}

inline std::string getMkid(const std::string &KIDPath) {
	return std::to_string(std::stoi(KIDPath.substr(0, 2)));
}

inline int64_t getUnixTime() {
	return static_cast<int64_t>(std::time(nullptr));
}

enum class KStat {
	active,
	disabled,
	revoked
};

struct KIDEntry {
	std::string label, note, b64;
	int64_t created;
	KStat status;

	// KIDEntry(std::string label, std::string note, KStat status = KStat::active): label(label), note(note), status(status) {}
	operator bool()
	{
		return (status == KStat::active);
	}

	static std::string statusSTR(const KStat &stat)
	{
		switch (stat)
		{
		case KStat::active:
			return "active";
		default:
			std::runtime_error("KIDEntry::statusSTR()::switch");
		}
	}
};

struct MKEntryB64 {
	std::string salt, nonce, ct;
	bool status = true;
	MKEntryB64() {}
	MKEntryB64(bool status) : status(status) {}
	operator bool() { return status; }
};

namespace ca_ori {
	extern int argc;
	extern char **argv;
}

struct CryptoGCM {
	BIN cipher, tag;
};

// util
extern BIN deriveKey(const BIN &ikm, const std::string &info, size_t keyLen, const BIN &salt = BIN());
extern BIN randomBIN(size_t size);
extern CryptoGCM encAES256GCM(const BIN &key, const BIN &nonce, const BIN &text, const BIN &AAD = BIN(0));
extern BIN decAES256GCM(const BIN &key, const BIN &nonce, const BIN &text, const BIN &tag, const BIN &AAD = BIN(0));
extern json readJson(const std::string &path);
extern void writeJson(const json &j, const std::string &path);
extern std::wstring to_wstring(const std::string &u8);

// MK
extern MKEntryB64 createMKCore(const std::string &pass, BIN mk = randomBIN(32));
extern BIN loadMKCore(const std::string &pass, const MKEntryB64 &res);
extern void createMK(int index, const std::string &pass, BIN mk = randomBIN(32));
extern BIN loadMK(int index, const std::string &pass);

// KID
extern void saveKID(const BIN &mk, const uint8_t &mkid, const ordered_json &body);
extern json loadKID(const BIN &mk, const uint8_t &mkid);
extern void addNewKid(ordered_json &body, const KIDEntry &kid_e);

// KEK
extern json createRawKEK(const BIN &mk, json kek_json, const json &kid_json, const uint8_t &mkid);
extern json encAdmKEK(const BIN &mk, const json &raw_json, const uint8_t &mkid);
extern json decAdmKEK(const BIN &mk, const json &adm_json);
extern json encPKEK(const json &raw_json);
extern json decPKEK(const json &p_json);
extern json encDstKEK(const std::string &password, const json &raw_json, unsigned long long opslimit = crypto_pwhash_OPSLIMIT_MODERATE, size_t memlimit = crypto_pwhash_MEMLIMIT_MODERATE);
extern json decDstKEK(const std::string &password, const json &dst_json);

// Core (DEK.cpp)
namespace EAD7 {
	extern BIN enc(const BIN &kek, const BIN &plaintext, const uint8_t &mkid, const BIN &kid);
	extern BIN dec(const BIN &kek, const BIN &blob);
	extern void encFile(const BIN &kek, const std::string &path, const uint8_t &mkid, const BIN &kid, uint32_t chunkSize);
	extern std::vector<uint64_t> decFile(const BIN &kek, const std::string &path);
}

// token
extern void saveToken(const BIN &token);
extern BIN loadToken();

// admin view
extern void adminUI();
