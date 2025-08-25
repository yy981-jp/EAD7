#include "def.h"
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>
#include <cryptopp/hkdf.h>

#include <string>


BIN deriveKey(const BIN &ikm, const std::string &info, size_t keyLen) {
    BIN derived(keyLen);

    HKDF<SHA256> hkdf;
    // saltはnullptrで長さ0にして省略
    hkdf.DeriveKey(derived, derived.size(), ikm, ikm.size(), nullptr, 0, 
                   reinterpret_cast<const byte*>(info.data()), info.size());

    return derived;
}
