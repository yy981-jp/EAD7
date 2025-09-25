#include "master.h"


BIN deriveDEC(const BIN& kek, const BIN& nonce) {
	std::string info = "EAD7|DEC|v1";
	return deriveKey(kek, info, 32, nonce);
}

CryptoGCM encCore(const BIN& kek, const BIN& plaintext, const BIN& aad, const BIN& nonce) {
	BIN decKey = deriveDEC(kek, nonce);

	CryptoGCM out = encAES256GCM(decKey, nonce, plaintext, aad);

	delm(decKey);
	return out;
}

BIN decCore(const BIN& kek, const BIN& nonce, const BIN& cipher, const BIN& aad, const BIN& tag) {
	BIN decKey = deriveDEC(kek, nonce);
	BIN plain = decAES256GCM(decKey, nonce, cipher, aad, tag);

	delm(decKey);
	return plain;
}

static BIN buildAAD(byte magic, byte ver, uint8_t mkid_i, const BIN& kid, const BIN& nonce) {
	size_t len = 1 + 1 + 1 + kid.size() + nonce.size(); // magic+ver+mkid+kid+nonce
	BIN aad(len);
	size_t p = 0;
	aad[p++] = magic;
	aad[p++] = ver;
	aad[p++] = mkid_i;
	if (kid.size()) memcpy(aad.data() + p, kid.data(), kid.size());
	p += kid.size();
	if (nonce.size()) memcpy(aad.data() + p, nonce.data(), nonce.size());
	// p += nonce.size();
	return aad;
}

namespace EAD7 {
	BIN enc(const BIN& kek, const BIN& plaintext, const uint8_t& mkid_i, const BIN& kid) {
		BIN nonce = randomBIN(12);
		BIN mkid(&mkid_i,1);
		if (kid.size() != HEADER::kid) throw std::runtime_error("kid must be 16 bytes");

		size_t total_size = 1 + 1 + 1 + HEADER::kid + HEADER::nonce + plaintext.size()/*=r.cipher.size()*/ + HEADER::tag;

		BIN result(total_size);
		size_t pos = 0;

		result[pos++] = HEADER::magicData;
		result[pos++] = HEADER::verData;
		memcpy(result.data() + pos, mkid.data(), mkid.size());
		pos += mkid.size();
		memcpy(result.data() + pos, kid.data(), kid.size());
		pos += kid.size();
		memcpy(result.data() + pos, nonce.data(), nonce.size());
		pos += nonce.size();
		
		CryptoGCM r = encCore(kek, plaintext, result/*この地点ではaad(header)部分のみ*/, nonce);
		
		// ct
		if (r.cipher.size()) {
			memcpy(result.data() + pos, r.cipher.data(), r.cipher.size());
			pos += r.cipher.size();
		} else std::runtime_error("::enc()::r.cipehr.size==0");

		// tag
		memcpy(result.data() + pos, r.tag.data(), r.tag.size());
		pos += r.tag.size();

		if (pos != total_size) throw std::runtime_error("size mismatch in enc");

		return result;
	}

	bool dec(const BIN& kek, const BIN& blob, BIN& out_plain) {
		if (blob.size() < HEADER::all) return false;

		// 切り出し
		size_t pos = 0;
		byte magic = blob[pos++];
		byte ver = blob[pos++];
		byte mkid = blob[pos++];
		BIN kid(blob.data() + pos, 16);  pos += 16;
		BIN nonce(blob.data() + pos, 12); pos += 12;
		BIN aad(blob.data(), pos); // 先頭〜nonceまで
		size_t cipher_size = blob.size() - pos - 16;
		BIN cipher(blob.data() + pos, cipher_size);
		pos += cipher_size;
		BIN tag(blob.data() + pos, 16);

		if (magic != HEADER::magicData) return false; // 明らかに違うデータ
		if (ver != HEADER::verData) return false; // 未対応バージョン

		// ct 長さ = 全体 - pos - HEADER::tag
		if (blob.size() < pos + HEADER::tag) return false; // 不正長

		try {
			out_plain = decCore(kek, cipher, aad, nonce, tag); // decCore が例外か失敗を返す想定
		} catch (...) {
			return false; // 認証失敗や復号失敗
		}

		return true;
	}
}