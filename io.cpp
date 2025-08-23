#include <string>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "def.h"
#include "master.h"

using json = nlohmann::json;


BIN readMK(const std::string& path, int index, const std::string& pass) {
	std::string sindex = std::to_string(index);
	std::ifstream ifile(path);
	if (!ifile) throw std::runtime_error("readMK()::ifstream");
	json jsondat;
	ifile >> jsondat;
	json entryj = jsondat.at(sindex);
	
	MKEntryB64 entry;
	entry.salt = entryj.at("salt").get<std::string>();
	entry.nonce = entryj.at("nonce").get<std::string>();
	entry.ct = entryj.at("ct").get<std::string>();
	return readMKCore(pass,entry);
}

void createMK(const std::string& path, int index, const std::string& pass) {
	std::string sindex = std::to_string(index);
	if (!fs::exists(path)) {
		std::ofstream ofile(path);
		if (!ofile) throw std::runtime_error("createMK()::ini::ofstream");
		ofile << "{}";
	}
	std::ifstream ifile(path);
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
	
	std::ofstream ofile(path);
	if (!ofile) throw std::runtime_error("createMK()::ofstream");
	ofile << jsondat;
}
