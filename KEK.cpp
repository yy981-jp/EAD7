#include "def.h"
#include <iostream>
#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

#include "master.h"
#include "base.h"


BIN HKDF_KEK(const BIN& mk, const std::string& KID) {
	std::string infoKEK = "EAD7|KEK|" + KID;
	return deriveKey(mk, infoKEK, 32);
}
BIN HKDF_ADMKEK(const BIN& mk, const BIN& salt) {
	std::string infoKEK = "EAD7|ADM-KEK";
	return deriveKey(mk, infoKEK, 32, salt);
}

json convert_kid_kek(const BIN& mk, const json& kid_json) {
	int64_t unix_now = static_cast<int64_t>(std::time(nullptr));
	json keks = json::object();

	for (auto& [kid_b64, entry] : kid_json["kids"].items()) {
		std::string label  = entry.value("label", "");
		std::string status = entry.value("status", "active");
		std::string note   = entry.value("note", "");
		int64_t created	= entry.value("created", unix_now);

		std::string kek_b64 = base::enc64(HKDF_KEK(mk,kid_b64));

		keks[kid_b64]["label"] = label;
		keks[kid_b64]["status"] = status;
		keks[kid_b64]["note"] = note;
		keks[kid_b64]["created"] = created;
		keks[kid_b64]["kek"] = kek_b64;
	}

	return keks;
}

json createRawKEK(const BIN& mk, json kek_json, const json& kid_json) { // kek_json:全体 kid_json:bodyのみ
	int64_t unix_now = static_cast<int64_t>(std::time(nullptr));
	if (!kek_json.contains("version")) {
		kek_json["meta"]["created"] = unix_now;
		kek_json["keks"] = json::object();
	};
	
	json raw = {
		{"version",1},
		{"type","raw"},
		{"meta", {
				{"created", kek_json["meta"]["created"].get<int64_t>()},
				{"last_updated", unix_now}
			}
		},
		{"keks",kek_json["keks"]}
	};
	raw["keks"].update(convert_kid_kek(mk,kid_json));
	return raw;
}
