#include "def.h"
#include <iostream>
#include <filesystem>
#include <fstream>

#include <cryptopp/hkdf.h>
#include <cryptopp/sha.h>

#include <nlohmann/json.hpp>

#include "master.h"


using json = nlohmann::json;


// HKDFで鍵を生成


BIN createKEKCore(const BIN& mk, const std::string& KID) {
    std::string infoKEK = "EAD7|KEK|" + KID;
    BIN KEK = deriveKey(mk, infoKEK, 32);
    // std::cout << "KEK: " << base::encHex(KEK) << std::endl;
	return KEK;
}

void writeKEK(const json& entry, int mkid) {
	const std::string path = SD + std::to_string(mkid) + ".kek.e7";
	
	if (!fs::exists(path)) {
		std::ofstream ofile(path);
		if (!ofile) throw std::runtime_error("writeKEK()::ini::ofstream");
		ofile << "{}";
	}
	
	std::ifstream ifile(path);
	if (!ifile) throw std::runtime_error("writeKEK()::ifstream");
	json j;
	ifile >> j;
	
	j["body"].update(entry);
	
	// HMAC処理ここに入れる
	
	std::ofstream ofile(path);
	if (!ofile) throw std::runtime_error("writeKEK()::ofstream");
	ofile << j;
}

bool createKEK(const BIN& mk, const std::string& KIDPath) {
	std::ifstream ifile(SD + getMkid(KIDPath) + ".kid.e7");
	if (!ifile) throw std::runtime_error("createKEK()::ifstream");
	json j;
	ifile >> j;
	for (auto& [key, value] : j["body"].items()) {
		if (value["name"] == KIDPath.substr(2)) {
			std::cout << "DEBUG: " << key << std::endl;
		}
	}
	return false;
}
/*
{
    // MK (32Bランダム例)
    // BIN MK(32);
    // for (size_t i = 0; i < MK.size(); ++i) MK[i] = i; // サンプル値
	// std::cout << "MK:  " << base::encHex(MK) << "\n";

    // KIDやNonce
    std::string KID = "KID-EXAMPLE-1234"; // 例
    std::string Nonce = "RANDOM-NONCE12";  // 例12バイト

    // 1️⃣ KEK生成
    // std::string infoKEK = "EAD7|KEK|" + KID;
    // BIN KEK = deriveKey(MK, infoKEK, 32);
    // std::cout << "KEK: " << base::encHex(KEK) << std::endl;

    // 2️⃣ DEK生成
    std::string infoDEK = "EAD7|DEK|" + Nonce;
    BIN DEK = deriveKey(KEK, infoDEK, 32);
    std::cout << "DEK: " << base::encHex(DEK) << std::endl;

    return 0;
}
*/