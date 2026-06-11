#pragma once
#include "def/bin.h"

#include <sodium.h>
#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>


struct CryptoGCM {
	Bin cipher, tag;
};


class AES256GCM {
	bool AES_NI;

	CryptoGCM enc_sodium(const Bin& key, const Bin& nonce, const Bin& plaintext, const Bin& aad) {
		CryptoGCM result;
		result.cipher.resize(plaintext.size());
		result.tag.resize(16);

		crypto_aead_aes256gcm_encrypt_detached(
			result.cipher.data(), result.tag.data(), nullptr,
			plaintext.data(), plaintext.size(),
			aad.data(), aad.size(),  // AAD
			nullptr, nonce.data(), key.data()
		);

		return result;
	}

	Bin dec_sodium(const Bin& key, const Bin& nonce, const Bin& ciphertext, const Bin& aad, const Bin& tag) {
		Bin plaintext(cipher.size());
		if (crypto_aead_aes256gcm_decrypt_detached(
				plaintext, NULL,
				cipher, cipher.size(),
				tag,
				aad.data(), aad.size(),	  // AAD
				nonce.data(),
				key.data()) != 0) {
			throw std::runtime_error("Decryption failed: tag mismatch or corrupted data");
		}

		return plaintext;
	}

	CryptoGCM enc_cryptopp(const Bin& key, const Bin& nonce, const Bin& plaintext, const Bin& aad) {
		CryptoGCM result;
		GCM<AES>::Encryption enc;
		enc.SetKeyWithnonce(key, key.size(), nonce, nonce.size());

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

	Bin dec_cryptopp(const Bin& key, const Bin& nonce, const Bin& ciphertext, const Bin& aad, const Bin& tag) {
		Bin recovered;
		GCM<AES>::Decryption dec;
		dec.SetKeyWithnonce(key, key.size(), nonce, nonce.size());

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


public:
	AES256GCM(bool AES_NI): AES_NI(AES_NI) {}

	CryptoGCM enc(const Bin& key, const Bin& nonce, const Bin& plaintext, const Bin& aad) {
		if (AES_NI) return enc_sodium(key, nonce, plaintext, aad);
			else	return enc_cryptopp(key, nonce, plaintext, aad);
	}

	Bin decAES256GCM(const Bin& key, const Bin& nonce, const Bin& text, const Bin& tag, const Bin& AAD) {
		if (AESN_I) return decAES256GCM_sodium(key, nonce, text, AAD, tag);
			else	return decAES256GCM_cryptopp(key, nonce, text, AAD, tag);
	}

};
