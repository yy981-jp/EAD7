#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include <yy981/proc.h>

#include "../master.h"
#include "../base.h"
#include "ui.h"
#include "../GUI/ui.h"


json getAdmKEK() {
	std::cout << "[存在するADM.KEK]\n";
	for (const fs::directory_entry& x : fs::directory_iterator(SDM)) {
		if (x.path().string().ends_with(".adm.kek.e7")) std::cout << x.path().string() << "\n";
	}
	std::string iname = inp("対象ADM.KEKファイルの名前(拡張子無し): ");
	return readJson(SDM+iname+"adm.kek.e7");
}

json loadKIDEntry(const json& kid) { //mkid 1つずつのみ対応 増やしたかったらその時作る?
	std::vector<Entry> list_i;
	for (const auto& [key,value]: kid["kids"].items()) {
		list_i.emplace_back(Entry(value.at("label"),key));
	}
	const std::vector<std::string> r = selectItem(list_i);
	json result; // kids部分に相当
	for (const std::string& e: r) {
		result.update(kid["keks"].at(e));
	}
	return result;
}

namespace uim {
	void MK() {
		json MKw = readJson(path::MK);
		
		std::string index;
		std::cout << "存在するMKのID: ";
		for (auto [key,value]: MKw.items()) {
			std::cout << key << " ";
			index += key;
		}
		std::cout << "\n";
		switch (choice("操作内容 (終了:E,作成:C,追加:I,解読:R,ファイルを開く:O)","ECIRO")) {
			case 'E': return;
			case 'C': {
				uint8_t mkid = std::stoi(inp("新しいMKのインデックス: "));
				if (!(mkid>=0 || mkid<=255)) {std::cerr << "MKIDは0~255である必要があります"; return;}
				std::string pass = inp("新しいMKのパスワード: ");
				::createMK(mkid,pass);
				delm(pass);
			} break; case 'I': {
				uint8_t mkid = std::stoi(inp("追加するMKのインデックス: "));
				if (!(mkid>=0 || mkid<=255)) {std::cerr << "MKIDは0~255である必要があります"; return;}
				std::string pass = inp("追加するMKのパスワード: ");
				std::string mk_b64 = inp("追加するMK(base64): ");
				BIN mk = base::dec64(mk_b64);
				::createMK(mkid,pass,mk);
				delm(mk,mk_b64);
			} break; case 'R': {
				uint8_t mkid = choice("対象MKID(候補="+index+"): ",index) - '0';
				std::string pass = inp("対象MKのパスワード: ");
				BIN mk = loadMK(mkid,pass);
				std::string mk_b64 = base::enc64(mk);
				std::cout << "生MK(Base64): " << mk_b64 << "\n";
				delm(mk,mk_b64);
			} break; case 'O': proc::start(path::MK); break;
		}
	}
	
	
	void KEK_C() { // kekを生成し、ADM.kekで保存
		uint8_t mkid = std::stoi(inp("対象KIDリストのMKID: "));
		if (!(mkid>=0 || mkid<=255)) {std::cerr << "MKIDは0~255である必要があります"; return;}
		std::string pass = inp("MKIDのMKのパスワード: ");
		BIN mk = loadMK(mkid,pass);
		json kid = loadKID(mk,mkid);
		json raw_kek = createRawKEK(mk,{},kid,mkid);
		json adm_kek = encAdmKEK(mk,raw_kek,mkid);
		delm(mk,raw_kek);
		std::string oname = inp("保存KEKリストファイル(***.adm.kek.e7)の名前(拡張子無し): ");
		writeJson(SDM+oname+"adm.kek.e7",adm_kek);
		delm(pass);
	}
	
	
	void KEK_M() {
		
	}
	
	void DST() {
		json adm_kek = getAdmKEK();
		uint8_t mkid = adm_kek["meta"]["mkid"].get<uint8_t>();
		std::string pass = inp("対象のADMに使用されたMKID("+std::to_string(mkid)+")のMKのパスワード: ");
		BIN mk = loadMK(mkid,pass);
		std::string dst_pass = inp("DST.KEKファイルのパスワード: ");
		json dst_kek = encDstKEK(dst_pass,decAdmKEK(mk,adm_kek));
		std::string oname = inp("配布KEKリストファイル(***.dst.kek.e7)の名前(拡張子無し): ");
		fs::path opath = fs::current_path()/oname/".dst.kek.e7";
		writeJson(opath.string(),dst_kek);
		delm(mk,dst_pass);
	}
}

void mmain() {
	fs::create_directories(SDM+"keks/");
	while (true) {
		try {
			char i = choice("[EAD7管理画面]\n1. MK管理\n2. KEK生成\n3. KEK管理\n4. DST.KEK生成", "");
			switch (i-'0') {
				case 1: uim::MK(); break;
				case 2: uim::KEK_C(); break;
				case 4: uim::DST(); break;
			}
			std::cout << "\n\n\n";
		} catch (std::runtime_error& err) {
			std::cout << "R_ERR: " << err.what() << "\n";
		}
	}
}
