#include <iostream>
#include <string>
#include <vector>
#include <cryptopp/cryptlib.h>
#include <cryptopp/hkdf.h>
#include <cryptopp/sha.h>
#include <cryptopp/secblock.h>

#include <yy981/string.h>

using namespace CryptoPP;

#include "base.h"
#include "master.h"

// HKDFで鍵を生成
SecByteBlock deriveKey(const SecByteBlock &ikm, const std::string &info, size_t keyLen) {
    SecByteBlock derived(keyLen);

    HKDF<SHA256> hkdf;
    // saltはnullptrで長さ0にして省略
    hkdf.DeriveKey(derived, derived.size(), ikm, ikm.size(), nullptr, 0, 
                   reinterpret_cast<const byte*>(info.data()), info.size());

    return derived;
}

int amain(int argc, char* argv[]) {
	// for (const KIDList& e: parseKIDList("../test.kid.m.txt")) std::cout << e.dat << "|" << e.ex << "\n";
	
	
	return 100;
	if (argc==2 || is_or(argv[1],"manage","master")) {
		mmain();
		return 981;
	}
    // MK (32Bランダム例)
    SecByteBlock MK(32);
    for (size_t i = 0; i < MK.size(); ++i) MK[i] = i; // サンプル値
	std::cout << "MK:  " << base::enHex(MK) << "\n";

    // KIDやNonce
    std::string KID = "KID-EXAMPLE-1234"; // 例
    std::string Nonce = "RANDOM-NONCE12";  // 例12バイト

    // 1️⃣ KEK生成
    std::string infoKEK = "EAD7|KEK|" + KID;
    SecByteBlock KEK = deriveKey(MK, infoKEK, 32);
    std::cout << "KEK: " << base::enHex(KEK) << std::endl;

    // 2️⃣ DEK生成
    std::string infoDEK = "EAD7|DEK|" + Nonce;
    SecByteBlock DEK = deriveKey(KEK, infoDEK, 32);
    std::cout << "DEK: " << base::enHex(DEK) << std::endl;

    return 0;
}
