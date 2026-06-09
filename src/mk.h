#pragma once
#include <string>
#include "crypto.h"

struct MKEntryB64 {
	std::string salt, nonce, ct;
	bool status = true;
	MKEntryB64() {}
	MKEntryB64(bool status) : status(status) {}
	explicit operator bool() const { return status; }
};

MKEntryB64 createMKCore(const std::string &pass, BIN mk = randomBIN(32));
BIN loadMKCore(const std::string &pass, const MKEntryB64 &res);
void createMK(int index, const std::string &pass, BIN mk = randomBIN(32));
BIN loadMK(int index, const std::string &pass);
