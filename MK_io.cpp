#include <string>
#include <fstream>
#include <filesystem>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>

#include "def.h"
#include "master.h"


BIN loadMK(int index, const std::string& pass) {
	std::string sindex = std::to_string(index);
	json jsondat;
	if (fs::exists(path::MK)) {
		std::ifstream ifile(path::MK);
		if (!ifile) throw std::runtime_error("loadMK()::ifstream");
		ifile >> jsondat;
	}
	if (!jsondat.contains(sindex)) return BIN(0);

	json entryj = jsondat.at(sindex);

	MKEntryB64 entry;
	entry.salt = entryj.at("salt").get<std::string>();
	entry.nonce = entryj.at("nonce").get<std::string>();
	entry.ct = entryj.at("ct").get<std::string>();
	return loadMKCore(pass,entry);
}

void createMK(int index, const std::string& pass, BIN mk) {
	std::string sindex = std::to_string(index);
	if (!fs::exists(path::MK)) {
		std::ofstream ofile(path::MK);
		if (!ofile) throw std::runtime_error("createMK()::ini::ofstream");
		ofile << "{}";
	}
	std::ifstream ifile(path::MK);
	if (!ifile) throw std::runtime_error("createMK()::ifstream");
	json jsondat;
	ifile >> jsondat;
	ifile.close();
	if (jsondat.contains(sindex)) throw std::runtime_error("インデックスに既にMKが存在するので処理を終了しました 削除などの編集はMK管理画面から行ってください");
	
	MKEntryB64 entry = createMKCore(pass);
	jsondat[sindex] = {
		{"salt",entry.salt},
		{"nonce",entry.nonce},
		{"ct",entry.ct}
	};
	
	std::ofstream ofile(path::MK);
	if (!ofile) throw std::runtime_error("createMK()::ofstream");
	ofile << jsondat;
}
