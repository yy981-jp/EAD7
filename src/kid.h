#pragma once
#include <string>
#include <utility>
#include "crypto.h"

enum class KStat {
	active,
	disabled,
	revoked
};

struct KIDEntry {
	std::string label, note, b64;
	int64_t created;
	KStat status;

	operator bool() const {
		return (status == KStat::active);
	}

	static std::string statusSTR(const KStat &stat);
};

std::pair<std::string, ordered_json> makeKidEntry(const KIDEntry &kid_e);
void addNewKid(ordered_json &body, const KIDEntry &kid_e);
void saveKID(const BIN &mk, const uint8_t &mkid, const ordered_json &body);
json loadKID(const BIN &mk, const uint8_t &mkid);
