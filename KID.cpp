#include "def.h"
#include <fstream>
#include <sodium.h>

#include <nlohmann/json.hpp>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include "base.h"
#include "master.h"

using ordered_json = nlohmann::ordered_json;

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


ordered_json loadKID(const int& mkid, const BIN& MK) {
	bool check = true;
	const std::string path = sdm + std::to_string(mkid) + ".kid.e7";
	if (!fs::exists(path)) {
		std::ofstream ofile(path);
		if (!ofile) throw std::runtime_error("writeKEK()::ini::ofstream");
		ofile << "{\"hmac\":\"\",\"body\":{\"version\":1,\"kids\":{}}}";
	}
	std::ifstream ifs(path);
	if (!ifs) throw std::runtime_error("KIDファイルを開けません: " + path);

	ordered_json j;
	ifs >> j;

	if (!j.contains("hmac") || !j.contains("body")) throw std::runtime_error("KIDファイル形式不正: hmac/body 不足");

	std::string hmac_b64 = j.at("hmac").get<std::string>();
	ordered_json body = j.at("body");
	
	if (!check) return body;

	// HMAC再計算
	BIN hkey = deriveKidlistHmacKey(MK);
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

void saveKIDconst (const int &mkid, const ordered_json& body, const BIN& MK) {
	const std::string path = sdm + std::to_string(mkid) + ".kid.e7";
	// HMAC計算
	BIN hkey = deriveKidlistHmacKey(MK);
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
	ofs << j.dump(0); // 最小化（HMACはbody.dump()基準なので外側の整形は自由）
}
