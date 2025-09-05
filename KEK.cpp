#include "def.h"
#include <iostream>
#include <filesystem>
#include <fstream>

#include "master.h"
#include "base.h"


json convert_kid_kek(const BIN& mk, const json& kid_json) {
	int64_t unix_now = static_cast<int64_t>(std::time(nullptr));
	json keks = json::object();

	for (auto& [kid_b64, entry] : kid_json["kids"].items()) {
		std::string label  = entry.value("label", "");
		std::string status = entry.value("status", "active");
		std::string note   = entry.value("note", "");
		int64_t created	= entry.value("created", unix_now);

		std::string infoKEK = "EAD7|KEK|v1|" + kid_b64;
		std::string kek_b64 = base::enc64(deriveKey(mk, infoKEK, 32));

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



json createAdmKEK(const BIN& mk, const json& raw_json) {
	if (!raw_json.is_object()) throw std::runtime_error("adm_json must be an object");

	int64_t unix_now = static_cast<int64_t>(std::time(nullptr));

	json adm;
	adm["version"] = raw_json.value("version", 1);
	adm["type"] = std::string("adm");
	adm["meta"] = {
		{"created", raw_json.at("created")},
		{"last_updated", unix_now}
	};
	adm["keks"] = json::object();
	
	for (auto& [kid_b64, entry] : raw_json["keks"].items()) {
		std::string label  = entry.at("label");
		std::string status = entry.at("status");
		int64_t created    = entry.at("created");
		// kek plain is in raw -> "kek"
		if (!entry.contains("kek")) throw std::runtime_error("raw entry missing kek for kid: " + kid_b64);
		std::string kek_b64 = entry["kek"].get<std::string>();
		BIN kek_plain = base::dec64(kek_b64);

		// 1) make per-entry salt (16B)
		BIN salt = randomBIN(16);

		// 2) derive entry key: HKDF(MK, salt, "EAD7|ADM|v1|"+kid, 32)
		std::string info = std::string("EAD7|ADM|v1|") + kid_b64;
		BIN entry_key = deriveKey(mk, info, 32, salt);

		// 3) prepare AAD: ordered json of fields (kid,label,status,created)
		ordered_json aad_obj;
		aad_obj["kid"] = kid_b64;
		aad_obj["label"] = label;
		aad_obj["status"] = status;
		aad_obj["created"] = created;
		std::string aad_str = aad_obj.dump(); // deterministic ordered dump
		BIN aad_bin(reinterpret_cast<const byte*>(aad_str.data()), aad_str.size());

		// 4) nonce (12B)
		BIN nonce = randomBIN(12);

		// 5) encrypt kek_plain with entry_key, nonce, aad_bin
		CryptoGCM cg = encAES256GCM(entry_key, nonce, kek_plain, aad_bin);

		// 6) store into adm JSON (base64 encode binary pieces)
		json adm_entry;
		adm_entry["label"] = label;
		adm_entry["status"] = status;
		adm_entry["created"] = created;
		adm_entry["salt"] = base::enc64(salt);
		adm_entry["enc"] = {
			{"ct", base::enc64(cg.cipher)},
			{"tag", base::enc64(cg.tag)},
			{"nonce", base::enc64(nonce)}
		};

		adm["keks"][kid_b64] = adm_entry;

		// 7) zero sensitive memory
		delm(kek_plain);
		delm(entry_key);
	}

	return adm;
}

json decAdmKEK(const BIN& mk, const json& adm_json) {
	if (!adm_json.is_object()) throw std::runtime_error("adm_json must be an object");

	int64_t unix_now = static_cast<int64_t>(std::time(nullptr));

	json raw;
	raw["version"] = adm_json.value("version", 1);
	raw["type"] = std::string("raw");
	raw["meta"] = {
		{"created", adm_json.at("created")},
		{"last_updated", unix_now}
	};
	raw["keks"] = json::object();

	for (auto& [kid_b64, adm_entry] : adm_json["keks"].items()) {
		// read fields
		std::string label  = adm_entry.at("label");
		std::string status = adm_entry.at("status");
		int64_t created    = adm_entry.at("created");

		BIN salt = base::dec64(adm_entry["salt"].get<std::string>());

		// derive entry key: HKDF(MK, salt, "EAD7|ADM|v1|"+kid, 32)
		std::string info = std::string("EAD7|ADM|v1|") + kid_b64;
		BIN entry_key = deriveKey(mk, info, 32, salt);

		// build AAD (ordered, deterministic)
		ordered_json aad_obj;
		aad_obj["kid"] = kid_b64;
		aad_obj["label"] = label;
		aad_obj["status"] = status;
		aad_obj["created"] = created;
		std::string aad_str = aad_obj.dump();
		BIN aad_bin(reinterpret_cast<const byte*>(aad_str.data()), aad_str.size());

		// extract enc fields
		json enc = adm_entry["enc"];
		BIN cipher = base::dec64(enc.at("ct"));
		BIN tag    = base::dec64(enc.at("tag"));
		BIN nonce  = base::dec64(enc.at("nonce"));

		// decrypt
		BIN kek_plain = decAES256GCM(entry_key, nonce, cipher, aad_bin, tag);

		// encode kek_plain back to base64 for raw format
		std::string kek_b64 = base::enc64(kek_plain);

		// build raw entry
		json raw_entry;
		raw_entry["label"] = label;
		raw_entry["status"] = status;
		raw_entry["created"] = created;
		raw_entry["kek"] = kek_b64;

		raw["keks"][kid_b64] = raw_entry;

		// zero sensitive memory
		delm(kek_plain);
		delm(entry_key);
		delm(salt);
		delm(cipher);
		delm(tag);
		delm(nonce);
	}

	return raw;
}
