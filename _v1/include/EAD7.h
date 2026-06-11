#pragma once
#include "../src/base.h"

namespace EAD7 {
	extern BIN enc(const BIN& kek, const BIN& plaintext, const uint8_t& mkid, const BIN& kid);
	extern BIN dec(const BIN& kek, const BIN& blob);
	extern void encFile(const BIN &kek, const std::string &path, const uint8_t &mkid, const BIN &kid, uint32_t chunkSize);
	extern std::vector<uint64_t> decFile(const BIN &kek, const std::string &path);
}
