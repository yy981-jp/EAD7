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
				uint8_t mkid = cmkid(inp("新しいMKのインデックス: "));
				std::string pass = inp_s("新しいMKのパスワード: ");
				::createMK(mkid,pass);
				delm(pass);
			} break;
			case 'I': {
				uint8_t mkid = cmkid(inp("追加するMKのインデックス(MKID): "));
				std::string pass = inp_s("追加するMKのパスワード: ");
				std::string mk_b64 = inp_s("追加するMK(base64): ");
				BIN mk = base::dec64(mk_b64);
				::createMK(mkid,pass,mk);
				delm(mk,mk_b64);
			} break;
			case 'R': {
				uint8_t mkid = choice("対象MKID(候補="+index+"): ",index) - '0';
				std::string pass = inp_s("対象MKのパスワード: ");
				BIN mk = loadMK(mkid,pass);
				std::string mk_b64 = base::enc64(mk);
				out_s("生MK(Base64): " + mk_b64 + "\n");
				delm(mk,mk_b64);
			} break;
			case 'O': openFile(path::MK); break;
		}
	}
	
	void KID() {
		switch (choice("操作内容 (終了:E,作成:C,追加:I,HMAC再計算:S,ファイルを開く:O)","ECISO")) {
			case 'E': return;
			case 'C': {
				uint8_t mkid = cmkid(inp("対象のKIDのMKID: "));
				std::string mkpass = inp_s("mkのパスワード: ");
				BIN mk = loadMK(mkid,mkpass);
				ordered_json j = loadKID(mk,mkid);
				KIDEntry kidEntry;
				kidEntry.label = inp("追加するKIDのラベル名: ");
				kidEntry.note = inp("追加するKIDの備考: ");
				kidEntry.status = KStat::active;
				addNewKid(j,kidEntry);
				saveKID(mk,mkid,j);
				delm(mk,mkpass);
			} break;
			case 'I': {
				uint8_t mkid = cmkid(inp("対象のKIDのMKID: "));
				std::string mkpass = inp_s("mkのパスワード: ");
				BIN mk = loadMK(mkid,mkpass);
				ordered_json j = loadKID(mk,mkid);
				KIDEntry kidEntry;
				kidEntry.label = inp("追加するKIDのラベル名: ");
				kidEntry.note = inp("追加するKIDの備考: ");
				kidEntry.b64 = inp("追加するKIDのbase64UrlSafe文字列: ");
				kidEntry.status = KStat::active;
				addNewKid(j,kidEntry);
				saveKID(mk,mkid,j);
				delm(mk,mkpass);
			} break;
			case 'S': {
				uint8_t mkid = cmkid(inp("対象のKIDのMKID: "));
				ordered_json j = readJson(SDM+std::to_string(mkid)+".kid.e7")["body"];
				const std::string pass = inp_s("対象MKIDのパスワード: ");
				BIN mk = loadMK(mkid,pass);
				saveKID(mk,mkid,j);
			} break;
			case 'O': {
				uint8_t mkid = cmkid(inp("対象のKIDのMKID: "));
				openFile(SDM+std::to_string(mkid)+".kid.e7");
			} break;
		}
	}
	
	
	void KEK_C() { // kekを生成し、ADM.kekで保存
		uint8_t mkid = cmkid(inp("対象KIDリストのMKID: "));
		std::string pass = inp_s("MKIDのMKのパスワード: ");
		BIN mk = loadMK(mkid,pass);
		json kid = loadKID(mk,mkid);
		json raw_kek = createRawKEK(mk,{},kid["kids"],mkid);
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
		std::string pass = inp_s("対象のADMに使用されたMKID("+std::to_string(mkid)+")のMKのパスワード: ");
		BIN mk = loadMK(mkid,pass);
		std::string dst_pass = inp_s("DST.KEKファイルのパスワード: ");
		json dst_kek = encDstKEK(dst_pass,decAdmKEK(mk,adm_kek));
		std::string oname = inp("配布KEKリストファイル(***.dst.kek.e7)の名前(拡張子無し): ");
		fs::path opath = fs::current_path()/oname/".dst.kek.e7";
		writeJson(opath.string(),dst_kek);
		delm(mk,dst_pass);
	}
}

void adminUI() {
	fs::create_directories(SDM+"keks/");
	while (true) {
		try {
			char i = choice("[EAD7管理画面]\nE. 終了\n1. MK管理\n2. KIDリスト管理\n3. KEK生成\n4. KEK管理\n5. DST.KEK生成\n", "E12345");
			switch (i) {
				case 'E': return;
				case '1': uim::MK(); break;
				case '2': uim::KID(); break;
				case '3': uim::KEK_C(); break;
				case '5': uim::DST(); break;
				
				default: throw std::runtime_error("adminUI()::switch");
			}
			std::cout << "\n\n\n";
		} catch (std::runtime_error& err) {
			std::cout << "R_ERR:\t" << err.what() << "\n\n\n\n";
		}
	}
}
