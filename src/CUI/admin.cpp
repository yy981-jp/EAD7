#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include <yy981/proc.h>

#include "master.h"
#include "base.h"
#include "ui.h"
#include "GUI/cui.h"
#include "UI/util.h"


json getAdmKEK(const bool embed = false) {
	std::cout << "[存在するADM.KEK]\n";
	for (const fs::directory_entry& x : fs::directory_iterator(SDMK)) {
		if (x.path().string().ends_with(".adm.kek.e7")) std::cout << "\t" << x.path().stem().stem().stem().string() << "\n";
	}
	std::string iname = inp("対象ADM.KEKファイルの名前(拡張子無ぁE: ");
	std::string path = getAdmKEKPath(iname);
	json j = readJson(path);
	if (!embed) return j;
	j["embedded_AdmKEKPath"] = path;
	return j;
}

json selectKIDEntry(const json& kid) { //mkid 1つずつのみ対忁E増やしたかったらそ�E時作る?     kid全体を受け取り、E��択されたエントリだけで構築されたkidのkids部刁E�Eみ返す
	std::vector<Entry> list_i;
	for (const auto& [key,value]: kid["kids"].items()) {
		list_i.emplace_back(Entry(value.at("label"),key));
	}
	const std::vector<std::string> r = selectItem(list_i);
	json result; // kids部刁E��相彁E
	for (const std::string& e: r) {
		result[e] = kid["kids"].at(e);
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
		switch (choice("操作�E容 (終亁EE,作�E:C,追加:I,解読:R,ファイルを開ぁEO)","ECIRO")) {
			case 'E': return;
			case 'C': {
				uint8_t mkid = cmkid(inp("新しいMKのインチE��クス: "));
				std::string pass = inp_s("新しいMKのパスワーチE ");
				::createMK(mkid,pass);
				delm(pass);
			} break;
			case 'I': {
				uint8_t mkid = cmkid(inp("追加するMKのインチE��クス(MKID): "));
				std::string pass = inp_s("追加するMKのパスワーチE ");
				std::string mk_b64 = inp_s("追加するMK(base64): ");
				BIN mk = base::dec64(mk_b64);
				::createMK(mkid,pass,mk);
				delm(mk_b64);
			} break;
			case 'R': {
				uint8_t mkid = choice("対象MKID(候裁E"+index+"): ",index) - '0';
				std::string pass = inp_s("対象MKのパスワーチE ");
				BIN mk = loadMK(mkid,pass);
				std::string mk_b64 = base::enc64(mk);
				out_s("生MK(Base64): " + mk_b64 + "\n");
				delm(mk_b64);
			} break;
			case 'O': openFile(path::MK); break;
		}
	}
	
	void KID() {
		switch (choice("操作�E容 (終亁EE,作�E:C,追加:I,HMAC再計箁ES,ファイルを開ぁEO)","ECISO")) {
			case 'E': return;
			case 'C': {
				uint8_t mkid = cmkid(inp("対象のKIDのMKID: "));
				std::string mkpass = inp_s("mkのパスワーチE ");
				BIN mk = loadMK(mkid,mkpass);
				ordered_json j = loadKID(mk,mkid);
				KIDEntry kidEntry;
				kidEntry.label = inp("追加するKIDのラベル吁E ");
				kidEntry.note = inp("追加するKIDの備老E ");
				kidEntry.status = KStat::active;
				addNewKid(j,kidEntry);
				saveKID(mk,mkid,j);
				delm(mkpass);
			} break;
			case 'I': {
				uint8_t mkid = cmkid(inp("対象のKIDのMKID: "));
				std::string mkpass = inp_s("mkのパスワーチE ");
				BIN mk = loadMK(mkid,mkpass);
				ordered_json j = loadKID(mk,mkid);
				KIDEntry kidEntry;
				kidEntry.label = inp("追加するKIDのラベル吁E ");
				kidEntry.note = inp("追加するKIDの備老E ");
				kidEntry.b64 = inp("追加するKIDのbase64UrlSafe斁E���E: ");
				kidEntry.status = KStat::active;
				addNewKid(j,kidEntry);
				saveKID(mk,mkid,j);
				delm(mkpass);
			} break;
			case 'S': {
				uint8_t mkid = cmkid(inp("対象のKIDのMKID: "));
				ordered_json j = readJson(getKIDFilePath(mkid))["body"];
				std::string pass = inp_s("対象MKIDのパスワーチE ");
				BIN mk = loadMK(mkid,pass);
				saveKID(mk,mkid,j);
				delm(pass);
			} break;
			case 'O': {
				uint8_t mkid = cmkid(inp("対象のKIDのMKID: "));
				openFile(getKIDFilePath(mkid));
			} break;
		}
	}
	
	void KEK() {
		switch (choice("操作�E容 (終亁EE,新規作�E:C,全体一覧:F,エントリ削除:D,エントリ追加:A)","ECFDA")) {
			case 'E': return;
			case 'C': {
				uint8_t mkid = cmkid(inp("対象KIDリスト�EMKID: "));
				std::string pass = inp_s("MKIDのMKのパスワーチE ");
				BIN mk = loadMK(mkid,pass);
				json kid = loadKID(mk,mkid);
				json selectedKid = selectKIDEntry(kid);
				json raw_kek = createRawKEK(mk,{},selectedKid,mkid);
				json adm_kek = encAdmKEK(mk,raw_kek,mkid);
				delm(raw_kek);
				std::string oname = inp("保存KEKリストファイル(***.adm.kek.e7)の名前(拡張子無ぁE: ");
				writeJson(adm_kek,getAdmKEKPath(oname));
				delm(pass);
			} break;
			case 'F': {
				for (const fs::directory_entry& x : fs::directory_iterator(SDMK)) {
					if (x.path().string().ends_with(".adm.kek.e7")) std::cout << x.path().stem().stem().stem().string() << "\n";
				}
			} break;
			case 'D': {
				json adm_kek = getAdmKEK(true);
				KIDIndex index = createKIDIndex(adm_kek);
				std::cout << "[存在するエントリ]\n";
				for (const auto& [label,kid]: index) {
					std::cout << "\t" << label << "\n";
				}
				std::string in_label = inp("対象エントリ: ");
				adm_kek["meta"]["last_updated"] = getUnixTime();
				adm_kek["keks"].erase(index[in_label]);
				std::string path = adm_kek["embedded_AdmKEKPath"];
				adm_kek.erase("embedded_AdmKEKPath");
				writeJson(adm_kek,path);
			} break;
			case 'A': {
				// 中断
			}
		}
	}
	
	void DST() {
		json adm_kek = getAdmKEK();
		uint8_t mkid = adm_kek["meta"]["mkid"].get<uint8_t>();
		std::string pass = inp_s("対象のADMに使用されたMKID("+std::to_string(mkid)+")のMKのパスワーチE ");
		BIN mk = loadMK(mkid,pass);
		std::string dst_pass = inp_s("DST.KEKファイルのパスワーチE ");
		json dst_kek = encDstKEK(dst_pass,decAdmKEK(mk,adm_kek));
		std::string oname = inp("配布KEKリストファイル(***.dst.kek.e7)の名前(拡張子無ぁE: ");
		fs::path opath = fs::current_path()/oname;
		writeJson(dst_kek,opath.string()+".dst.kek.e7");
		// clear sensitive temporaries
		delm(dst_pass, adm_kek);
	}
}

void adminUI() {
	fs::create_directories(SDM+"keks/");
	while (true) {
		try {
			char i = choice("[EAD7管琁E��面]\nE. 終亁En1. MK管琁En2. KIDリスト管琁En3. KEK管琁En4. DST.KEK生�E\n", "E12345");
			switch (i) {
				case 'E': return;
				case '1': uim::MK(); break;
				case '2': uim::KID(); break;
				case '3': uim::KEK(); break;
				case '4': uim::DST(); break;
				
				default: throw std::runtime_error("adminUI()::switch");
			}
			std::cout << "\n";
		} catch (std::runtime_error& err) {
			std::cout << "R_ERR:\t" << err.what() << "\n\n";
		}
	}
}
