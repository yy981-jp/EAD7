#pragma once
#include <atomic>
#include <string>
#include <vector>
#include "def.h"
#include "file.h"

namespace EAD7 {
	BIN enc(const BIN &kek, const BIN &plaintext, const uint8_t &mkid, const BIN &kid);
	BIN dec(const BIN &kek, const BIN &blob);
	void encFile(const BIN &kek, const std::string &path, const uint8_t &mkid, const BIN &kid, uint32_t chunkSize, std::atomic<uint64_t> *currentChunkNumber = nullptr);
	std::vector<uint64_t> decFile(const BIN &kek, const std::string &path, std::atomic<uint64_t> *currentChunkNumber = nullptr);
}

FHeader getFileHeader(const std::string &path);
