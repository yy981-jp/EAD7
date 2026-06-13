#include <ead7/crypto/aes.h>

#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>
#include <sodium.h>


EAD7_BEGIN

using namespace CryptoPP;


CryptoGCM AES256GCM::enc_sodium(const Bin& key, const Bin& nonce, const Bin& plaintext, const Bin& aad) {
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

Bin AES256GCM::dec_sodium(const Bin& key, const Bin& nonce, const Bin& ciphertext, const Bin& aad, const Bin& tag) {
	Bin plaintext(ciphertext.size());
	if (crypto_aead_aes256gcm_decrypt_detached(
			plaintext, NULL,
			ciphertext, ciphertext.size(),
			tag,
			aad.data(), aad.size(),	  // AAD
			nonce.data(),
			key.data()) != 0) {
		throw std::runtime_error("Decryption failed: tag mismatch or corrupted data");
	}

	return plaintext;
}

CryptoGCM AES256GCM::enc_cryptopp(const Bin& key, const Bin& nonce, const Bin& plaintext, const Bin& aad) {
	CryptoGCM result;
	GCM<AES>::Encryption enc;
	enc.SetKeyWithIV(key, key.size(), nonce, nonce.size());

	// AADを先にProcessData
	enc.SpecifyDataLengths(aad.size(), plaintext.size(), 0);

	// 認証タグ生成にAADを加える
	enc.Update(aad.data(), aad.size());

	result.cipher.cleanNew(plaintext.size());
	enc.ProcessData(result.cipher, plaintext, plaintext.size());

	// 認証タグを取り出す
	result.tag.cleanNew(16);
	enc.TruncatedFinal(result.tag, result.tag.size());
	
	return result;
}

Bin AES256GCM::dec_cryptopp(const Bin& key, const Bin& nonce, const Bin& ciphertext, const Bin& aad, const Bin& tag) {
	Bin recovered;
	GCM<AES>::Decryption dec;
	dec.SetKeyWithIV(key, key.size(), nonce, nonce.size());

	// 平文サイズ指定
	dec.SpecifyDataLengths(aad.size(), ciphertext.size(), 0);

	// 認証タグにAADを加える
	dec.Update(aad.data(), aad.size());

	recovered.cleanNew(ciphertext.size());
	dec.ProcessData(recovered, ciphertext, ciphertext.size());

	// タグを検証
	if (!dec.TruncatedVerify(tag, tag.size())) throw std::runtime_error("Tag mismatch (AAD tampered?)");
	
	return recovered;
}


EAD7_END
