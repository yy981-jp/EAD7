#include "AES256GCM.h"

#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>


CryptoGCM encAES256GCM_sodium(const BIN& key, const BIN& nonce, const BIN& plaintext, const BIN& aad) {
	CryptoGCM result;
    result.cipher.resize(plaintext.size());
    result.tag.resize(16);

    unsigned long long clen = 0;
    if (crypto_aead_aes256gcm_is_available() == 0) {
        throw std::runtime_error("encAES256GCM_sodium()"); // そもそも利用不可
    }

	crypto_aead_aes256gcm_encrypt_detached(
		result.cipher.data(), result.tag.data(), nullptr,
		plaintext.data(), plaintext.size(),
		aad.data(), aad.size(),  // ←ここで AAD を渡す
		nullptr, nonce.data(), key.data()
	);

    return result;
}

BIN decAES256GCM_sodium(const BIN& key, const BIN& nonce, const BIN& cipher, const BIN& aad, const BIN& tag) {
    BIN plaintext(cipher.size());

    if (crypto_aead_aes256gcm_is_available() == 0) {
        throw std::runtime_error("AES-NI not available on this system");
    }

    unsigned long long plen = 0;
    if (crypto_aead_aes256gcm_decrypt_detached(
            plaintext, NULL,
            cipher, cipher.size(),
            tag,
            aad.data(), aad.size(),      // AAD
            nonce.data(),
            key.data()) != 0) {
        throw std::runtime_error("Decryption failed: tag mismatch or corrupted data");
    }

    return plaintext;
}



CryptoGCM encAES256GCM_cryptopp(const BIN& key, const BIN& iv, const BIN& plaintext, const BIN& aad) {
	CryptoGCM result;
	GCM<AES>::Encryption enc;
	enc.SetKeyWithIV(key, key.size(), iv, iv.size());

	// AADを先にProcessData
	enc.SpecifyDataLengths(aad.size(), plaintext.size(), 0);

	// 認証タグ生成にAADを加える
	enc.Update(aad.data(), aad.size());

	result.cipher.CleanNew(plaintext.size());
	enc.ProcessData(result.cipher, plaintext, plaintext.size());

	// 認証タグを取り出す
	result.tag.CleanNew(16);
	enc.TruncatedFinal(result.tag, result.tag.size());
	
	return result;
}

BIN decAES256GCM_cryptopp(const BIN& key, const BIN& iv, const BIN& ciphertext, const BIN& aad, const BIN& tag) {
	BIN recovered;
	GCM<AES>::Decryption dec;
	dec.SetKeyWithIV(key, key.size(), iv, iv.size());

	// 平文サイズ指定
	dec.SpecifyDataLengths(aad.size(), ciphertext.size(), 0);

	// 認証タグにAADを加える
	dec.Update(aad.data(), aad.size());

	recovered.CleanNew(ciphertext.size());
	dec.ProcessData(recovered, ciphertext, ciphertext.size());

	// タグを検証
	if (!dec.TruncatedVerify(tag, tag.size())) throw std::runtime_error("Tag mismatch (AAD tampered?)");
	
	return recovered;
}