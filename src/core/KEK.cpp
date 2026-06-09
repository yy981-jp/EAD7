#include <iostream>
#include <filesystem>
#include <fstream>

#include "kek.h"
#include "token.h"
#include "base.h"


BIN derivekey_password(const std::string& password, const BIN& salt,
					   size_t keyLen = 32,
					   unsigned long long opslimit = crypto_pwhash_OPSLIMIT_MODERATE,
					   size_t memlimit = crypto_pwhash_MEMLIMIT_MODERATE) {
	BIN key(keyLen);
	
	if (crypto_pwhash(key.data(), key.size(),
					  password.c_str(), password.size(),
					  salt.data(),
					  opslimit, memlimit,
					  crypto_pwhash_ALG_ARGON2ID13) != 0) {
		throw std::runtime_error("Password hashing failed (maybe out of memory)");
	}
	return key;
}

BIN deriveKEK(const BIN& mk, const std::string kid_b64) {
	std::string infoKEK = "EAD7|KEK|v1|" + kid_b64;
	return deriveKey(mk, infoKEK, 32);
}


json convert_kid_kek(const BIN& mk, const json& kid_json, const uint8_t& mkid) {
	int64_t unix_now = getUnixTime();
	json keks = json::object();

	for (auto& [kid_b64, entry] : kid_json.items()) {
		std::string label  = entry.value("label", "");
		std::string status = entry.value("status", "active");
		std::string note   = entry.value("note", "");
		int64_t created	= entry.value("created", unix_now);

		std::string kek_b64 = base::enc64(deriveKEK(mk, kid_b64));

		keks[kid_b64]["label"] = label;
		keks[kid_b64]["status"] = status;
		keks[kid_b64]["note"] = note;
		keks[kid_b64]["created"] = created;
		keks[kid_b64]["mkid"] = mkid;
		keks[kid_b64]["kek"] = kek_b64;
	}

	return keks;
}

json createRawKEK(const BIN& mk, json kek_json, const json& kid_json, const uint8_t& mkid) { // kek_json:蜈ｨ菴・kid_json:kids縺ｮ縺ｿ
	int64_t unix_now = getUnixTime();
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
	raw["keks"].update(convert_kid_kek(mk,kid_json,mkid));
	return raw;
}



json encAdmKEK(const BIN& mk, const json& raw_json, const uint8_t& mkid_adm) {
	if (!raw_json.is_object()) throw std::runtime_error("adm_json must be an object");

	int64_t unix_now = getUnixTime();

	json adm;
	adm["version"] = raw_json.at("version");
	adm["type"] = std::string("adm");
	adm["meta"] = {
		{"created", raw_json.at("meta").at("created")},
		{"last_updated", unix_now},
		{"mkid", mkid_adm}
	};
	adm["keks"] = json::object();
	
	for (auto& [kid_b64, entry] : raw_json["keks"].items()) {
		std::string label  = entry.at("label");
		std::string status = entry.at("status");
		int64_t created	= entry.at("created");
		uint8_t mkid = entry.at("mkid");
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
		aad_obj["mkid"] = mkid;
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
		adm_entry["mkid"] = mkid;
		adm_entry["salt"] = base::enc64(salt);
		adm_entry["enc"] = {
			{"ct", base::enc64(cg.cipher)},
			{"tag", base::enc64(cg.tag)},
			{"nonce", base::enc64(nonce)}
		};

		adm["keks"][kid_b64] = adm_entry;

		// 7) zero sensitive memory

	}

	return adm;
}

json decAdmKEK(const BIN& mk, const json& adm_json) {
	if (!adm_json.is_object()) throw std::runtime_error("adm_json must be an object");

	json raw;
	raw["version"] = adm_json.at("version");
	raw["type"] = std::string("raw");
	raw["meta"] = {
		{"created", adm_json.at("meta").at("created")},
		{"last_updated", adm_json.at("meta").at("last_updated")}
	};
	raw["keks"] = json::object();

	for (auto& [kid_b64, adm_entry] : adm_json["keks"].items()) {
		// read fields
		std::string label  = adm_entry.at("label");
		std::string status = adm_entry.at("status");
		int64_t created	= adm_entry.at("created");
		uint8_t mkid = adm_entry.at("mkid");

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
		aad_obj["mkid"] = mkid;
		std::string aad_str = aad_obj.dump();
		BIN aad_bin(reinterpret_cast<const byte*>(aad_str.data()), aad_str.size());

		// extract enc fields
		json enc = adm_entry["enc"];
		BIN cipher = base::dec64(enc.at("ct"));
		BIN tag	= base::dec64(enc.at("tag"));
		BIN nonce  = base::dec64(enc.at("nonce"));

		// decrypt
		BIN kek_plain = decAES256GCM(entry_key, nonce, cipher, tag, aad_bin);

		// encode kek_plain back to base64 for raw format
		std::string kek_b64 = base::enc64(kek_plain);

		// build raw entry
		json raw_entry;
		raw_entry["label"] = label;
		raw_entry["status"] = status;
		raw_entry["created"] = created;
		raw_entry["mkid"] = mkid;
		raw_entry["kek"] = kek_b64;

		raw["keks"][kid_b64] = raw_entry;

		// zero sensitive memory

	}

	return raw;
}



json encPKEK(const json& raw_json) {
	int64_t unix_now = getUnixTime();

	// --- 1) AAD菴懈・ ---
	ordered_json aad_obj;
	aad_obj["version"] = raw_json.at("version");
	aad_obj["type"] = "p";
	aad_obj["meta"] = {
		{"created", raw_json.at("meta").at("created")},
		{"last_updated", unix_now}
	};
	std::string aad_str = aad_obj.dump();
	BIN aad_bin(reinterpret_cast<const byte*>(aad_str.data()), aad_str.size());

	// --- 2) 證怜捷蟇ｾ雎｡繝・・繧ｿ (raw_json["keks"])
	std::string keks_str = raw_json["keks"].dump();
	BIN keks_bin(reinterpret_cast<const byte*>(keks_str.data()), keks_str.size());

	// --- 3) salt 縺ｨ nonce 逕滓・
	BIN salt = randomBIN(16);
	BIN nonce = randomBIN(12);

	// --- 4) 豢ｾ逕滄嵯
	std::string info = "EAD7|KEK|v1";
	BIN file_key = deriveKey(loadToken(), info, 32, salt);

	// --- 5) AES256-GCM 縺ｧ證怜捷蛹・
	CryptoGCM cg;
	try {
		cg = encAES256GCM(file_key, nonce, keks_bin, aad_bin);
	} catch (const std::runtime_error& err) {
		std::cerr << "token縺却.kek縺檎ｴ謳阪＠縺ｦ縺・ｋ蜿ｯ閭ｽ諤ｧ縺碁ｫ倥＞縺ｧ縺・遘ｻ讀阪・豁｣隕上・謇矩・↓蜑・▲縺ｦ陦後▲縺ｦ縺上□縺輔＞";
		throw err;
	}
	// --- 6) JSON 蜃ｺ蜉・
	json p;
	p["version"] = aad_obj["version"];
	p["type"] = aad_obj["type"];
	p["meta"] = aad_obj["meta"];
	p["enc"] = {
		{"salt", base::enc64(salt)},
		{"ct", base::enc64(cg.cipher)},
		{"tag", base::enc64(cg.tag)},
		{"nonce", base::enc64(nonce)}
	};

	// --- 7) 繧ｻ繝ｳ繧ｷ繝・ぅ繝悶ョ繝ｼ繧ｿ蜑企勁


	return p;
}

json decPKEK(const json& p_json) {
	// --- 1) AAD 蜀肴ｧ狗ｯ・---
	ordered_json aad_obj;
	aad_obj["version"] = p_json.at("version");
	aad_obj["type"] = p_json.at("type");
	aad_obj["meta"] = p_json.at("meta");
	std::string aad_str = aad_obj.dump();
	BIN aad_bin(reinterpret_cast<const byte*>(aad_str.data()), aad_str.size());

	// --- 2) enc 驛ｨ蛻・ｒ蜿門ｾ・---
	if (!p_json.contains("enc")) {
		throw std::runtime_error("p.kek missing enc field");
	}
	auto enc = p_json["enc"];
	BIN salt  = base::dec64(enc.at("salt").get<std::string>());
	BIN ct	= base::dec64(enc.at("ct").get<std::string>());
	BIN tag   = base::dec64(enc.at("tag").get<std::string>());
	BIN nonce = base::dec64(enc.at("nonce").get<std::string>());

	// --- 3) 骰ｵ繧貞ｰ主・ ---
	std::string info = "EAD7|KEK|v1";
	BIN file_key = deriveKey(loadToken(), info, 32, salt);

	// --- 4) 蠕ｩ蜿ｷ ---
	BIN plain;
	try {
		plain = decAES256GCM(file_key, nonce, ct, tag, aad_bin);
	} catch (const std::runtime_error& err) {
		std::cerr << "token縺却.kek縺檎ｴ謳阪＠縺ｦ縺・ｋ蜿ｯ閭ｽ諤ｧ縺碁ｫ倥＞縺ｧ縺・遘ｻ讀阪・豁｣隕上・謇矩・↓蜑・▲縺ｦ陦後▲縺ｦ縺上□縺輔＞";
		throw err;
	}

	// --- 5) 蟷ｳ譁・ｒ JSON 縺ｨ縺励※蠕ｩ蜈・---
	std::string plain_str(reinterpret_cast<const char*>(plain.data()), plain.size());
	json keks = json::parse(plain_str);

	// --- 6) 蜃ｺ蜉・JSON 讒狗ｯ・(raw 縺ｫ霑代＞蠖｢縺ｫ謌ｻ縺・
	json raw;
	raw["version"] = p_json.at("version");
	raw["type"] = "raw"; // decrypt蠕後・逕溘・KEK讒矩縺ｫ謌ｻ繧・
	raw["meta"] = p_json.at("meta");
	raw["keks"] = keks;

	// --- 7) 繧ｻ繝ｳ繧ｷ繝・ぅ繝悶ョ繝ｼ繧ｿ蜑企勁 ---


	return raw;
}



// p蠖｢蠑・ keks荳ｸ縺斐→證怜捷蛹・dst菴懈・
json encDstKEK(const std::string &password, const json &raw_json, unsigned long long opslimit, size_t memlimit) {
	if (!raw_json.is_object()) throw std::runtime_error("raw_json must be an object");

	int64_t unix_now = getUnixTime();

	// 繝輔ぃ繧､繝ｫ繝ｬ繝吶Ν縺ｮsalt逕滓・
	BIN file_salt = randomBIN(16);

	// password + file_salt 縺九ｉ fileKey繧貞ｰ主・
	const size_t KEY_LEN = 32;
	BIN fileKey = derivekey_password(password, file_salt, KEY_LEN, opslimit, memlimit);

	// keks繧剃ｸｸ縺斐→證怜捷蛹・
	std::string keks_str = raw_json.at("keks").dump();
	BIN keks_bin(reinterpret_cast<const byte*>(keks_str.data()), keks_str.size());

	// nonce逕滓・・医ヵ繧｡繧､繝ｫ蜊倅ｽ搾ｼ・
	BIN nonce = randomBIN(12);

	// AAD讒狗ｯ・(version/type/meta/kdf)
	ordered_json aad_obj;
	aad_obj["version"] = raw_json.at("version");
	aad_obj["type"] = "dst";
	aad_obj["meta"] = {
		{"created", raw_json.at("meta").at("created")},
		{"last_updated", unix_now}
	};
	aad_obj["kdf"] = {
		{"opslimit", (unsigned long long)opslimit},
		{"memlimit", (unsigned long long)memlimit},
		{"salt", base::enc64(file_salt)}
	};
	std::string aad_str = aad_obj.dump();
	BIN aad_bin(reinterpret_cast<const byte*>(aad_str.data()), aad_str.size());

	// 證怜捷蛹・
	CryptoGCM cg = encAES256GCM(fileKey, nonce, keks_bin, aad_bin);

	// dst JSON讒狗ｯ・
	json dst;
	dst["version"] = raw_json.at("version");
	dst["type"] = "dst";
	dst["meta"] = {
		{"created", raw_json.at("meta").at("created")},
		{"last_updated", unix_now}
	};
	dst["kdf"] = {
		{"opslimit", (unsigned long long)opslimit},
		{"memlimit", (unsigned long long)memlimit},
		{"salt", base::enc64(file_salt)}
	};
	dst["enc"] = {
		{"ct", base::enc64(cg.cipher)},
		{"tag", base::enc64(cg.tag)},
		{"nonce", base::enc64(nonce)}
	};

	// 繧ｼ繝ｭ蛹・


	return dst;
}

// p蠖｢蠑・ keks荳ｸ縺斐→蠕ｩ蜿ｷ
json decDstKEK(const std::string &password, const json &dst_json) {
	if (sodium_init() < 0) throw std::runtime_error("sodium_init failed");
	if (!dst_json.is_object()) throw std::runtime_error("dst_json must be an object");

	int64_t unix_now = getUnixTime();

	// kdf 諠・ｱ
	if (!dst_json.contains("kdf") || !dst_json["kdf"].is_object()) {
		throw std::runtime_error("dst_json missing kdf object");
	}
	auto kdf = dst_json["kdf"];
	unsigned long long opslimit = kdf.at("opslimit");
	size_t memlimit = static_cast<size_t>(kdf.at("memlimit"));
	std::string salt_b64 = kdf.at("salt");
	if (salt_b64.empty()) throw std::runtime_error("kdf.salt missing");

	BIN file_salt = base::dec64(salt_b64);
	const size_t KEY_LEN = 32;
	BIN fileKey = derivekey_password(password, file_salt, KEY_LEN, opslimit, memlimit);

	// 讒矩讀懈渊
	if (!dst_json.contains("enc") || !dst_json["enc"].is_object()) {

		throw std::runtime_error("dst_json missing 'enc' object");
	}
	json enc = dst_json["enc"];
	std::string ct_b64 = enc.at("ct");
	std::string tag_b64 = enc.at("tag");
	std::string nonce_b64 = enc.at("nonce");

	BIN cipher = base::dec64(ct_b64);
	BIN tag	= base::dec64(tag_b64);
	BIN nonce  = base::dec64(nonce_b64);

	// AAD讒狗ｯ会ｼ・st菴懈・譎ゅ→蜷梧ｧ假ｼ・
	ordered_json aad_obj;
	aad_obj["version"] = dst_json.at("version");
	aad_obj["type"] = dst_json.at("type");
	aad_obj["meta"] = {
		{"created", dst_json.at("meta").at("created")},
		{"last_updated", dst_json.at("meta").at("last_updated")}
	};
	aad_obj["kdf"] = {
		{"opslimit", dst_json.at("kdf").at("opslimit")},
		{"memlimit", dst_json.at("kdf").at("memlimit")},
		{"salt", dst_json.at("kdf").at("salt")}
	};
	std::string aad_str = aad_obj.dump();
	BIN aad_bin(reinterpret_cast<const byte*>(aad_str.data()), aad_str.size());

	// 蠕ｩ蜿ｷ
	BIN keks_bin;
	try {
		keks_bin = decAES256GCM(fileKey, nonce, cipher, tag, aad_bin);
	} catch (const std::exception &e) {

		throw std::runtime_error(std::string("dst decryption failed: ") + e.what());
	}

	// JSON縺ｫ謌ｻ縺・
	std::string keks_str(reinterpret_cast<const char*>(keks_bin.data()), keks_bin.size());
	json raw_keks = json::parse(keks_str);

	json raw;
	raw["version"] = dst_json.at("version");
	raw["type"] = "raw";
	raw["meta"] = {
		{"created", dst_json.at("meta").at("created")},
		{"last_updated", unix_now}
	};
	raw["keks"] = raw_keks;

	// 繧ｼ繝ｭ蛹・


	return raw;
}
