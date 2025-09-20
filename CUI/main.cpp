#include <iostream>
#include <map>
#include <functional>
#include <stdexcept>

#include <yy981/string.h>

#include "ui.h"
#include "text.h"
#include "../master.h"
#include "../base.h"


bool interactive = false;
bool UISwitch_failed = false;

enum class KEKIndexType {
	label, kid
};

using KEKIndex = std::map<std::string,std::string>;

KEKIndex createKEKIndex(const json& j, KEKIndexType t) {
	KEKIndex result;
	for (auto& [key,value]: j["keks"].items()) {
		switch (t) {
			case KEKIndexType::label: result[value["label"]] = result["kek"]; break;
			case KEKIndexType::kid: result[key] = value["kek"]; break;
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
	
	
	
	void encrypt() {
		KEKIndex index = createKEKIndex(decPKEK(readJson(SD+"kek.e7")),KEKIndexType::label);
		
	}

	void decrypt() {
		
	}
	
	void f_info() {
		const std::string dat = inp("対象のファイルパス,base64,json: ");
		FDat f = getFileType(dat);
		switch (f.type) {
			case FSType::MK: {
				std::cout << "[MK]:マスターキーリスト\n存在するMKID: ";
				for (auto [key,value]: f.json.items()) {
					std::cout << key << ",";
				}
				std::cout << "\nより詳細な情報は管理者モードで起動して操作してください\n";
			} break;
			case FSType::kid: {
				json j = loadKid(f.json);
				std::cout << "[kid]:KIDリスト\n"
				for (auto [key,v]: j["keks"]) {
					std::cout << v["label"] << " {"
							  << "\n\tID: " << key
							  << "\n\t作成日時: " << convUnixTime(v["created"].get<int64_t>())
							  << "\n\t状態: " << v["status"]
							  << "\n}";
				}
			} break;
			case FSType::raw_kek: {
				std::cout << "[raw.kek]:生KEK"
						  << "\n作成日時: " << convUnixTime(f.json["meta"]["created"].get<int64_t>())
						  << "\n最終更新日時: " << convUnixTime(f.json["meta"]["last_updated"].get<int64_t>())
				for (auto [key,v]: f.json["keks"].items()) {
					std::cout << "\n\t" << v["label"] << " {"
							  << "\n\t\tID: " << key
							  << "\n\t\t状態: " << v["status"]
							  << "\n\t\t作成日時: " << v["created"]
							  << "\n\t}";
				}
				delm(f.json);
			} break;
			case FSType::p_kek: {
				json j = decPKEK(f.json);
				std::cout << "[p.kek]:通常KEK"
						  << "\n作成日時: " << convUnixTime(j["meta"]["created"].get<int64_t>())
						  << "\n最終更新日時: " << convUnixTime(j["meta"]["last_updated"].get<int64_t>())
				for (auto [key,v]: j["keks"].items()) {
					std::cout << "\n\t" << v["label"] << " {"
							  << "\n\t\tID: " << key
							  << "\n\t\t状態: " << v["status"]
							  << "\n\t\t作成日時: " << v["created"]
							  << "\n\t}";
				}
				delm(j);
			} break;
			case FSType::cus_kek: {
				// 後で実装するはず 未来の自分よろ
				exit(1000);
			} break;
			case FSType::adm_kek: {
				std::cout << "[p.kek]:通常KEK"
						  << "\n作成日時: " << convUnixTime(f.json["meta"]["created"].get<int64_t>())
						  << "\n最終更新日時: " << convUnixTime(f.json["meta"]["last_updated"].get<int64_t>())
				for (auto [key,v]: f.json["keks"].items()) {
					std::cout << "\n\t" << v["label"] << " {"
							  << "\n\t\tID: " << key
							  << "\n\t\t状態: " << v["status"]
							  << "\n\t\t作成日時: " << v["created"]
							  << "\n\t}";
				}
			}
		}
	}
}

std::map<std::vector<std::string>, std::function<void()>> commands = {
	{{"list","l","L"}, ui::list},
	{{"help","h","H"}, ui::help},
	{{"encrypt","enc","en","e"}, ui::encrypt},
	{{"decrypt","dec","de","d"}, ui::decrypt}
};

void UI() { // UIの実質main関数
	switch (ca.size()) {
		case 1: interactive = true; break;
		case 2: case 3: {
			for (auto [aliases,handler]: commands) {
				if (is_or(ca[1],aliases)) {
					handler();
					return;
				}
			}
			UISwitch_failed = true;
		} break;
		default: UISwitch_failed = true; break;
	}
	if (UISwitch_failed) throw std::runtime_error("CLI引数エラー \"EAD7 h\"を参照してください");
}
