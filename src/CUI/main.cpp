#include <iostream>
#include <map>
#include <functional>
#include <stdexcept>

#include <yy981/string.h>

#include "ui.h"
#include "text.h"
#include "../master.h"
#include "../base.h"
#include "../GUI/CUI.h"
#include "../UI/info.h"


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
			delm(raw_kek);
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
			// clear sensitive buffers (keep non-BIN items)
			delm(raw_kek);
	}
	
	void f_info3() {
		FDat f = getFileType(fs::path(ca[2]));
		std::cout << getFileInfo(false, f);
	}
	
	void f_info2() {
		const std::string dat = inp("対象のファイルパス,base64,json: ");
		FDat f = getFileType(dat);
		std::cout << getFileInfo(false, f);
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
	{{"encrypt","enc","en","e"}, ui::encrypt}
};


int UI() { // CUIの実質main関数
	try {
		switch (ca.size()) {
			case 1: return GUI_interface();
			case 2: {
				if (ui::dst()) return 0;
				for (auto [aliases,handler]: commands2) {
					if (is_or(ca[1],aliases)) {
						handler();
						return 0;
					}
				}
				throw std::runtime_error("CLI引数エラー argc2");
			} break;
			case 3: {
				for (auto [aliases,handler]: commands3) {
					if (is_or(ca[1],aliases)) {
						handler();
						return 0;
					}
				}
				throw std::runtime_error("CLI引数エラー argc3");
			} break;
			case 4: {
				for (auto [aliases,handler]: commands4) {
					if (is_or(ca[1],aliases)) {
						handler();
						return 0;
					}
				}
				throw std::runtime_error("CLI引数エラー argc4");
			} break;
			default: throw std::runtime_error("CLI引数エラー 0層"); break;
		}
	} catch (const std::runtime_error& err) {
		std::cout << "R_ERR:\t" << err.what() << "\n";
	}
	return 1;
}
