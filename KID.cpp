#include "def.h"
#include <fstream>
#include <sodium.h>

#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include <yy981/return.h>

#include "base.h"
#include "master.h"


// HMAC-SHA256(key: 32B) → 32B
inline BIN hmac_sha256(const BIN& key, const std::string& data) {
	HMAC<SHA256> h(key.data(), key.size());
	BIN mac(32);
	h.Update(reinterpret_cast<const unsigned char*>(data.data()), data.size());
	h.Final(mac.data());
	return mac;
}

// HMACKey = HKDF(MK, "EAD7|KIDLIST-HMAC") の派生
inline BIN deriveKidlistHmacKey(const BIN& MK) {
	return deriveKey(MK, "EAD7|KIDLIST-HMAC", 32);
}



// KIDエントリJSON（label/created/status/note）
std::pair<std::string,ordered_json> makeKidEntry(const KIDEntry& kid_e) {
	ordered_json j;
	std::string b64 = base::enc64(randomBIN(16));
	j["label"] = kid_e.label;
	j["created"] = static_cast<int64_t>(std::time(nullptr));
	j["status"] = kid_e.status; // "active" | "disabled" | "revoked"
	j["note"] = kid_e.note;
	return {b64,j};
}

void addNewKid(ordered_json& body, const KIDEntry& kid_e) {
	for (auto& [key, val] : body["kids"].items()) if (val["label"] == kid_e.label) return_e("すでに同じラベルのKIDが存在します");
	std::pair<std::string,ordered_json> entry = makeKidEntry(kid_e);
	body["kids"][entry.first] = entry.second;
}



ordered_json loadKID(const BIN& mk, const int& mkid) {
	if (mk.empty()) throw std::runtime_error("loadKID()::MKが破損しています");
	const std::string path = SDM + std::to_string(mkid) + ".kid.e7";
	ordered_json j;
	if (!fs::exists(path)) {
		j = {
			{"hmac",""},
			{"body",{
				{"version",1},
					{"kids",ordered_json::object()}
				}
			}
		};
		return j["body"];
	} else {
		std::ifstream ifs(path);
		if (!ifs) throw std::runtime_error("KIDファイルを開けません: " + path);
		ifs >> j;
	}

	if (!j.contains("hmac") || !j.contains("body")) throw std::runtime_error("KIDファイル形式不正: hmac/body 不足");

	std::string hmac_b64 = j.at("hmac").get<std::string>();
	ordered_json body = j.at("body");
	
	// HMAC再計算
	BIN hkey = deriveKidlistHmacKey(mk);
	std::string body_dump = body.dump();
	BIN mac = hmac_sha256(hkey, body_dump);

	// 後始末（MK→hkeyを消す）
	delm(hkey);

	// 比較
	BIN mac_stored = base::dec64(hmac_b64);
	if (mac_stored.size() != mac.size() || 0 != std::memcmp(mac_stored.data(), mac.data(), mac.size()))
		throw std::runtime_error("KIDファイルHMAC検証失敗（改ざん or MK不一致）");

	return body;
}

void saveKID(const BIN& mk, const int &mkid, const ordered_json& body) {
	if (mk.empty()) throw std::runtime_error("saveKID()::MKが破損しています");
	const std::string path = SDM + std::to_string(mkid) + ".kid.e7";
	// HMAC計算
	BIN hkey = deriveKidlistHmacKey(mk);
	std::string body_dump = body.dump();
	BIN mac = hmac_sha256(hkey, body_dump);
	delm(hkey);

	// JSON組み立て
	ordered_json j;
	j["body"] = body;
	j["hmac"] = base::enc64(mac);

	// 保存
	std::ofstream ofs(path);
	if (!ofs) throw std::runtime_error("KIDファイルを書き込めません: " + path);
	ofs << j;
}
