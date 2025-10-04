#include <iostream>
#include <map>
#include <functional>
#include <stdexcept>

#include <yy981/string.h>

#include "ui.h"
#include "text.h"
#include "../master.h"
#include "../base.h"


bool UISwitch_failed = false;

KIDIndex createKIDIndex(const json& j, KIDIndexType t) { // raw.kek必須
	KIDIndex result;
	for (auto& [key,value]: j["keks"].items()) {
		switch (t) {
			case KIDIndexType::label: result[value["label"]] = key; break;
			// case KEKIndexType::kid: result[key] = key; break;
		}
	}
	return result;
}

static void f_infoCore(FDat& f) {
	switch (f.type) {
		case FSType::MK: {
			std::cout << "[MK]:マスターキーリスト\n存在するMKID: ";
			for (auto [key,value]: f.json.items()) {
				std::cout << key << ",";
			}
			std::cout << "\nより詳細な情報は管理者モードで起動して操作してください\n";
		} break;
		case FSType::kid: {
			std::cout << "[kid]:KIDリスト\nより詳細な情報は管理者モードで起動して操作してください\n";
		} break;
		case FSType::raw_kek: {
			std::cout << "[raw.kek]:生KEK"
					  << "\n作成日時:     " << convUnixTime(f.json["meta"]["created"].get<int64_t>())
					  << "\n最終更新日時: " << convUnixTime(f.json["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: f.json["keks"].items()) {
				std::cout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状態: " << v["status"]
						  << "\n\t\t作成日時: " << convUnixTime(v["created"])
						  << "\n\t}";
			}
			delm(f.json);
		} break;
		case FSType::p_kek: {
			json j = decPKEK(f.json);
			std::cout << "[p.kek]:通常KEK"
					  << "\n作成日時:     " << convUnixTime(j["meta"]["created"].get<int64_t>())
					  << "\n最終更新日時: " << convUnixTime(j["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: j["keks"].items()) {
				std::cout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状態: " << v["status"]
						  << "\n\t\t作成日時: " << convUnixTime(v["created"])
						  << "\n\t}";
			}
			delm(j);
		} break;
		case FSType::cus_kek: {
			// 後で実装するはず 未来の自分よろ
			exit(1000);
		} break;
		case FSType::adm_kek: {
			std::cout << "[p.kek]:管理者KEK"
					  << "\n作成日時:     " << convUnixTime(f.json["meta"]["created"].get<int64_t>())
					  << "\n最終更新日時: " << convUnixTime(f.json["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: f.json["keks"].items()) {
				std::cout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状態: " << v["status"]
						  << "\n\t\t作成日時: " << convUnixTime(v["created"])
						  << "\n\t}";
			}
		} break;
		case FSType::dst_kek: {
			std::cout << "[dst.kek]:配布KEK\n";
			std::string pass = inp_s("パスワード: ");
			json j = decDstKEK(pass,f.json);
			std::cout << "\n作成日時:     " << convUnixTime(j["meta"]["created"].get<int64_t>())
					  << "\n最終更新日時: " << convUnixTime(j["meta"]["last_updated"].get<int64_t>());
			for (auto [key,v]: j["keks"].items()) {
				std::cout << "\n\t" << v["label"] << " {"
						  << "\n\t\tID: " << key
						  << "\n\t\t状態: " << v["status"]
						  << "\n\t\t作成日時: " << convUnixTime(v["created"])
						  << "\n\t}";
			}
		} break;
/*			case FSType::base64: {
			std::cout << "base64urlsafe\n";
		} break;
		case FSType::encBin: {
			std::cout << "[暗号文()]"
		} break;
*/			default: std::cout << "多分正しいe7系統データではない"; // 処理を実装してない部分だったらごめん
	}
}





namespace ui {
	void list() {
		
	}
	
	void help() {
		std::cout << t::banner << t::help;
	}
	
	void ver() {
		std::cout << t::banner << t::version;
	}
	
	
	
	void encrypt() {
		std::string label = ca[2];
		json raw_kek = decPKEK(readJson(SD+"kek.e7"));
		KIDIndex index = createKIDIndex(raw_kek);
		if (!index.contains(label)) throw std::runtime_error("ラベル("+label+")がこのPCのKEKリストに存在しません");
		std::string kid = index[label];
		json entry = raw_kek["keks"][kid];
		if (entry["status"] != "active") throw std::runtime_error("このKEKは現在有効ではありません");
		uint8_t mkid = entry["mkid"].get<uint8_t>();
		BIN kek = base::dec64(entry["kek"]);
		BIN plaintext = conv::STRtoBIN(ca[3]);
		BIN cipher = EAD7::enc(kek,plaintext,mkid,base::dec64(kid));
		std::cout << "暗号: " << base::enc64(cipher) << "\n";
		delm(raw_kek,kek);
	}

	void decrypt() {
		BIN cipher = base::dec64(ca[2]);
		json raw_kek = decPKEK(readJson(path::p_kek));
		EAD7ST es(cipher);
		json entry = raw_kek["keks"][base::enc64(es.kid)];
		if (entry.empty()) {
			std::cerr << "あなたはこのルームの鍵を持っていないため、解読できません\n";
			return;
		}
		BIN kek = base::dec64(entry["kek"]);
		BIN plaintext = EAD7::dec(kek,cipher);
		std::cout << "原文: " << conv::BINtoSTR(plaintext) << "\n";
	}
	
	void f_info3() {
		FDat f = getFileType(fs::path(ca[2]));
		f_infoCore(f);
	}
	
	void f_info2() {
		const std::string dat = inp("対象のファイルパス,base64,json: ");
		FDat f = getFileType(dat);
		f_infoCore(f);
	}
	
	bool dst() {
		if (ca[1].ends_with(".e7")) {
			fs::path p = ca[1];
			if (fs::exists(p)) {
				FDat f = getFileType(p);
				switch (f.type) {
					case FSType::dst_kek: {
						std::string pass = inp_s("DST.KEKファイルのパスワード: ");
						json raw_kek = decDstKEK(pass,f.json);
						json p_kek = encPKEK(raw_kek);
						writeJson(p_kek,path::p_kek);
						std::cout << "P.KEK更新完了\n";
						delm(pass,raw_kek);
					} break;
					default: throw std::runtime_error("E7ファイルではありますが、形式が不正です");
				}
				return true;
			}
		}
		return false;
	}
}

static std::map<std::vector<std::string>, std::function<void()>> commands2 = {
	{{"master","m","admin"}, adminUI},
	{{"list","l","L"}, ui::list},
	{{"help","h","H"}, ui::help},
	{{"ver","v","V"}, ui::ver},
	{{"info","i","I"}, ui::f_info2}
}, commands3 = {
	{{"info","i","I"}, ui::f_info3},
	{{"decrypt","dec","de","d"}, ui::decrypt}
}, commands4 = {
	{{"encrypt","enc","en","e"}, ui::encrypt},
};
/*
void CUI_ini() {
}
*/
void UI() { // CUIの実質main関数
	// CUI_ini();
	try {
		switch (ca.size()) {
			case 1: std::cout << "GUI未実装"; break;
			case 2: {
				if (ui::dst()) return;
				for (auto [aliases,handler]: commands2) {
					if (is_or(ca[1],aliases)) {
						handler();
						return;
					}
				}
				throw std::runtime_error("CLI引数エラー argc2");
			} break;
			case 3: {
				for (auto [aliases,handler]: commands3) {
					if (is_or(ca[1],aliases)) {
						handler();
						return;
					}
				}
				throw std::runtime_error("CLI引数エラー argc3");
			} break;
			case 4: {
				for (auto [aliases,handler]: commands4) {
					if (is_or(ca[1],aliases)) {
						handler();
						return;
					}
				}
				throw std::runtime_error("CLI引数エラー argc4");
			} break;
			default: throw std::runtime_error("CLI引数エラー 0層"); break;
		}
	} catch (const std::runtime_error& err) {
		std::cout << "R_ERR:\t" << err.what() << "\n";
	}
}
