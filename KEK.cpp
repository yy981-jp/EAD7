#include <iostream>
#include <filesystem>
#include <fstream>

#include "master.h"
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
	adm["version"] = raw_json.at("version");
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
	raw["version"] = adm_json.at("version");
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



json createPKEK(const json& raw_json) {
	int64_t unix_now = static_cast<int64_t>(std::time(nullptr));

	// --- 1) AAD作成 ---
	ordered_json aad_obj;
	aad_obj["version"] = raw_json.at("version");
	aad_obj["type"] = "p";
	aad_obj["meta"] = {
		{"created", raw_json["meta"].at("created")},
		{"last_updated", unix_now}
	};
	std::string aad_str = aad_obj.dump();
	BIN aad_bin(reinterpret_cast<const byte*>(aad_str.data()), aad_str.size());

	// --- 2) 暗号対象データ (raw_json["keks"])
	std::string keks_str = raw_json["keks"].dump();
	BIN keks_bin(reinterpret_cast<const byte*>(keks_str.data()), keks_str.size());

	// --- 3) salt と nonce 生成
	BIN salt = randomBIN(16);
	BIN nonce = randomBIN(12);

	// --- 4) 派生鍵
	std::string info = "EAD7|KEK|v1";
	BIN file_key = deriveKey(loadToken(), info, 32, salt);

	// --- 5) AES256-GCM で暗号化
	CryptoGCM cg = encAES256GCM(file_key, nonce, keks_bin, aad_bin);

	// --- 6) JSON 出力
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

	// --- 7) センシティブデータ削除
	delm(keks_bin);
	delm(file_key);

	return p;
}

json decPKEK(const json& p_json) {
	// --- 1) AAD 再構築 ---
	ordered_json aad_obj;
	aad_obj["version"] = p_json.at("version");
	aad_obj["type"] = p_json.at("type");
	aad_obj["meta"] = p_json.at("meta");
	std::string aad_str = aad_obj.dump();
	BIN aad_bin(reinterpret_cast<const byte*>(aad_str.data()), aad_str.size());

	// --- 2) enc 部分を取得 ---
	if (!p_json.contains("enc")) {
		throw std::runtime_error("p.kek missing enc field");
	}
	auto enc = p_json["enc"];
	BIN salt  = base::dec64(enc.at("salt").get<std::string>());
	BIN ct    = base::dec64(enc.at("ct").get<std::string>());
	BIN tag   = base::dec64(enc.at("tag").get<std::string>());
	BIN nonce = base::dec64(enc.at("nonce").get<std::string>());

	// --- 3) 鍵を導出 ---
	std::string info = "EAD7|KEK|v1";
	BIN file_key = deriveKey(loadToken(), info, 32, salt);

	// --- 4) 復号 ---
	BIN plain = decAES256GCM(file_key, nonce, ct, aad_bin, tag);

	// --- 5) 平文を JSON として復元 ---
	std::string plain_str(reinterpret_cast<const char*>(plain.data()), plain.size());
	json keks = json::parse(plain_str);

	// --- 6) 出力 JSON 構築 (raw に近い形に戻す)
	json raw;
	raw["version"] = p_json.at("version");
	raw["type"] = "raw"; // decrypt後は生のKEK構造に戻る
	raw["meta"] = p_json.at("meta");
	raw["keks"] = keks;

	// --- 7) センシティブデータ削除 ---
	delm(file_key);
	delm(plain);

	return raw;
}



json createDstKEK(const std::string &password, const json &raw_json, unsigned long long opslimit, size_t memlimit) {
	if (!raw_json.is_object()) throw std::runtime_error("raw_json must be an object");

	int64_t unix_now = static_cast<int64_t>(std::time(nullptr));

	// generate file-level salt
	BIN file_salt = randomBIN(crypto_pwhash_SALTBYTES); // 16B

	// derive fileKey from password + file_salt
	const size_t KEY_LEN = 32;
	BIN fileKey = derivekey_password(password, file_salt, opslimit, memlimit, KEY_LEN);

	// build dst json
	json dst;
	dst["version"] = raw_json.at("version");
	dst["type"] = std::string("dst");
	dst["kdf"] = {
		{"opslimit", (unsigned long long)opslimit},
		{"memlimit", (unsigned long long)memlimit},
		{"salt", base::enc64(file_salt)}
	};
	dst["meta"] = {
		{"created", raw_json.at("meta").at("created")},
		{"last_updated", unix_now}
	};
	dst["keks"] = json::object();

	// iterate entries in raw_json
	if (!raw_json.contains("keks") || !raw_json["keks"].is_object()) {
		throw std::runtime_error("raw_json missing 'keks' object");
	}

	for (auto & [kid_b64, entry] : raw_json["keks"].items()) {
		std::string label  = entry.at("label");
		std::string status = entry.at("status");
		int64_t created    = entry.at("created");

		if (!entry.contains("kek")) throw std::runtime_error("raw entry missing kek for kid: " + kid_b64);
		BIN kek_plain = base::dec64(entry["kek"].get<std::string>());

		// AAD: ordered json of kid,label,status,created and also kdf params (for tamper protection)
		ordered_json aad_obj;
		aad_obj["kid"] = kid_b64;
		aad_obj["label"] = label;
		aad_obj["status"] = status;
		aad_obj["created"] = created;
		// include KDF info in AAD so tampering with kdf params / salt is detectable
		aad_obj["kdf"] = {
			{"opslimit", (unsigned long long)opslimit},
			{"memlimit", (unsigned long long)memlimit},
			{"salt", base::enc64(file_salt)}
		};
		std::string aad_str = aad_obj.dump();
		BIN aad_bin(reinterpret_cast<const byte*>(aad_str.data()), aad_str.size());

		// nonce per-entry
		BIN nonce = randomBIN(12);

		// encrypt with fileKey
		CryptoGCM cg = encAES256GCM(fileKey, nonce, kek_plain, aad_bin);

		// build dst entry
		json dst_entry;
		dst_entry["label"] = label;
		dst_entry["status"] = status;
		dst_entry["created"] = created;
		dst_entry["enc"] = {
			{"ct", base::enc64(cg.cipher)},
			{"tag", base::enc64(cg.tag)},
			{"nonce", base::enc64(nonce)}
		};

		dst["keks"][kid_b64] = dst_entry;

		// zero
		delm(kek_plain);
		delm(nonce);
		delm(aad_bin);
		// note: keep fileKey until all entries done
	}

	// zero fileKey
	delm(fileKey);

	return dst;
}

json decDstKEK(const std::string &password, const json &dst_json) {
	if (sodium_init() < 0) throw std::runtime_error("sodium_init failed");
	if (!dst_json.is_object()) throw std::runtime_error("dst_json must be an object");

	int64_t unix_now = static_cast<int64_t>(std::time(nullptr));

	// kdf 情報の読み出し (name omitted per design)
	if (!dst_json.contains("kdf") || !dst_json["kdf"].is_object()) {
		throw std::runtime_error("dst_json missing kdf object");
	}
	auto kdf = dst_json["kdf"];
	unsigned long long opslimit = kdf.at("opslimit");
	size_t memlimit = static_cast<size_t>(kdf.at("memlimit"));
	std::string salt_b64 = kdf.at("salt");
	if (salt_b64.empty()) throw std::runtime_error("kdf.salt missing");

	// decode salt and derive fileKey
	BIN file_salt = base::dec64(salt_b64);
	const size_t KEY_LEN = 32;
	BIN fileKey = derivekey_password(password, file_salt, KEY_LEN, opslimit, memlimit);

	// 構造の検査
	if (!dst_json.contains("keks") || !dst_json["keks"].is_object()) {
		delm(fileKey);
		throw std::runtime_error("dst_json missing 'keks' object");
	}

	// 出力 raw skeleton
	json raw;
	raw["version"] = dst_json.at("version");
	raw["type"] = std::string("raw");
	raw["meta"] = {
		{"created", dst_json.at("meta").at("created")},
		{"last_updated", unix_now}
	};
	raw["keks"] = json::object();

	// 走査して各エントリを復号
	for (auto & [kid_b64, dst_entry] : dst_json["keks"].items()) {
		std::string label  = dst_entry.at("label");
		std::string status = dst_entry.at("status");
		int64_t created    = dst_entry.at("created");

		if (!dst_entry.contains("enc")) {
			delm(fileKey);
			throw std::runtime_error("dst entry missing enc for kid: " + kid_b64);
		}

		json enc = dst_entry["enc"];
		std::string ct_b64    = enc.at("ct");
		std::string tag_b64   = enc.at("tag");
		std::string nonce_b64 = enc.at("nonce");
		if (ct_b64.empty() || tag_b64.empty() || nonce_b64.empty()) {
			delm(fileKey);
			throw std::runtime_error("enc object missing fields for kid: " + kid_b64);
		}

		// base64 -> BIN
		BIN cipher = base::dec64(ct_b64);
		BIN tag    = base::dec64(tag_b64);
		BIN nonce  = base::dec64(nonce_b64);

		// AAD を再構築（ordered_json.dump() で決定的に）
		ordered_json aad_obj;
		aad_obj["kid"] = kid_b64;
		aad_obj["label"] = label;
		aad_obj["status"] = status;
		aad_obj["created"] = created;
		// kdf 情報 (salt/opslimit/memlimit) を AAD に含めて改ざん検知
		aad_obj["kdf"] = {
			{"opslimit", (unsigned long long)opslimit},
			{"memlimit", (unsigned long long)memlimit},
			{"salt", base::enc64(file_salt)}
		};
		std::string aad_str = aad_obj.dump();
		BIN aad_bin(reinterpret_cast<const byte*>(aad_str.data()), aad_str.size());

		// 復号
		BIN kek_plain;
		try {
			kek_plain = decAES256GCM(fileKey, nonce, cipher, aad_bin, tag);
		} catch (const std::exception &e) {
			// 敏感データを消してから投げ直す
			delm(fileKey);
			delm(cipher);
			delm(tag);
			delm(nonce);
			throw std::runtime_error(std::string("dst decryption failed for kid: ") + kid_b64 + " - " + e.what());
		}

		// raw 形式のエントリを組み立て（kek を base64 化）
		std::string kek_b64 = base::enc64(kek_plain);
		json raw_entry;
		raw_entry["label"] = label;
		raw_entry["status"] = status;
		raw_entry["created"] = created;
		raw_entry["kek"] = kek_b64;
		raw["keks"][kid_b64] = raw_entry;

		// 敏感データをゼロ化
		delm(kek_plain);
		delm(cipher);
		delm(tag);
		delm(nonce);
		delm(aad_bin);
	}

	// fileKey をゼロ化して返す
	delm(fileKey);
	return raw;
}

