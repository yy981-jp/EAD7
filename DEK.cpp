#include "master.h"


BIN deriveDEC(const BIN& kek, const BIN& nonce) {
	std::string info = "EAD7|DEC|v1";
	return deriveKey(kek, info, 32, nonce);
}

CryptoGCM_nonce encCore(const BIN& kek, const BIN& plaintext, const BIN& aad) {
	BIN nonce = randomBIN(12);
	BIN decKey = deriveDEC(kek, nonce);

	CryptoGCM_nonce result;
	CryptoGCM out = encAES256GCM(decKey, nonce, plaintext, aad);

	result.cipher = out.cipher;
	result.tag = out.tag;
	result.nonce = nonce;
	delm(decKey);
	return result;
}

BIN decCore(const BIN& kek, const BIN& nonce, const BIN& cipher, const BIN& aad, const BIN& tag) {
	BIN decKey = deriveDEC(kek, nonce);
	BIN plain = decAES256GCM(decKey, nonce, cipher, aad, tag);

	delm(decKey);
	return plain;
}

BIN enc(const BIN& kek, const BIN& plaintext, const BIN& aad, const BIN& mkid, const BIN& kid) {
	if (mkid.size() != 1) throw std::runtime_error("mkid must be 1 byte");
	if (kid.size() != 16) throw std::runtime_error("kid must be 16 bytes");

	CryptoGCM_nonce r = encCore(kek, plaintext, aad); // r.nonce:12, r.cipher:var, r.tag:16

	size_t total_size = 1 + 1 + mkid.size() + kid.size() + r.nonce.size() + r.cipher.size() + r.tag.size();

	BIN result(total_size);
	size_t pos = 0;

	result[pos++] = HEADER::magicData;
	result[pos++] = HEADER::verData;

	// mkid
	memcpy(result.data() + pos, mkid.data(), mkid.size());
	pos += mkid.size();

	// kid
	memcpy(result.data() + pos, kid.data(), kid.size());
	pos += kid.size();

	// nonce
	memcpy(result.data() + pos, r.nonce.data(), r.nonce.size());
	pos += r.nonce.size();

	// ct
	if (r.cipher.size()) {
		memcpy(result.data() + pos, r.cipher.data(), r.cipher.size());
		pos += r.cipher.size();
	}

	// tag
	memcpy(result.data() + pos, r.tag.data(), r.tag.size());
	pos += r.tag.size();

	if (pos != total_size) throw std::runtime_error("size mismatch in enc");

	return result;
}

bool dec(const BIN& kek, const BIN& blob, const BIN& aad, BIN& out_plain) {
	if (blob.size() < HEADER::all) return false;

	size_t pos = 0;

	if (blob[pos++] != HEADER::magicData) return false; // 明らかに違うデータ
	byte ver = blob[pos++];
	if (ver != HEADER::verData) return false; // 未対応バージョン

	// mkid
	// BIN mkid(&blob[pos], HEADER::mkid);
	pos += HEADER::mkid;

	// kid (16b)
	// BIN kid(blob.data() + pos, HEADER:kid);
	pos += HEADER::kid;

	// nonce (12b)
	BIN nonce(blob.data() + pos, HEADER::nonce);
	pos += HEADER::nonce;

	// ct 長さ = 全体 - pos - HEADER::tag
	if (blob.size() < pos + HEADER::tag) return false; // 不正長
	size_t ct_len = blob.size() - pos - HEADER::tag;

	BIN ct;
	if (ct_len) {
		ct = BIN(blob.data() + pos, ct_len);
		pos += ct_len;
	}

	// tag
	BIN tag(blob.data() + pos, HEADER::tag);
	pos += HEADER::tag;

	try {
		out_plain = decCore(kek, ct, aad, nonce, tag); // decCore が例外か失敗を返す想定
	} catch (...) {
		return false; // 認証失敗や復号失敗
	}

	return true;
}
