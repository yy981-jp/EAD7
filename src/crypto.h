#pragma once
#include <string>
#include <cstdint>
#include <ctime>
#include <string>
#include "def.h"
#include "file.h"

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
	constexpr uint64_t default_size = 1ULL * 1024 * 1024; // 1 MiB
	constexpr uint64_t min = 4ULL * 1024;                 // 4 KiB
	constexpr uint64_t max = 16ULL * 1024 * 1024;         // 16 MiB (safety upper bound)
}

std::string getMkid(const std::string &KIDPath);
int64_t getUnixTime();
std::string getKIDFilePath(const uint8_t &mkid);

struct CryptoGCM {
	BIN cipher, tag;
};

BIN deriveKey(const BIN &ikm, const std::string &info, size_t keyLen, const BIN &salt = BIN());
BIN randomBIN(size_t size);
CryptoGCM encAES256GCM(const BIN &key, const BIN &nonce, const BIN &text, const BIN &AAD = BIN(0));
BIN decAES256GCM(const BIN &key, const BIN &nonce, const BIN &text, const BIN &tag, const BIN &AAD = BIN(0));
json readJson(const std::string &path);
void writeJson(const json &j, const std::string &path);
std::wstring to_wstring(const std::string &u8);
