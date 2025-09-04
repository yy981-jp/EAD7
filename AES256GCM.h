#pragma once
#include "master.h"


extern CryptoGCM encAES256GCM_sodium(const BIN& key, const BIN& nonce, const BIN& ciphertext, const BIN& aad);
extern BIN decAES256GCM_sodium(const BIN& key, const BIN& nonce, const BIN& ciphertext, const BIN& aad, const BIN& tag);
extern CryptoGCM encAES256GCM_cryptopp(const BIN& key, const BIN& iv, const BIN& ciphertext, const BIN& aad);
extern BIN decAES256GCM_cryptopp(const BIN& key, const BIN& iv, const BIN& ciphertext, const BIN& aad, const BIN& tag);
