#include <iostream>
#include <map>
#include <functional>
#include <stdexcept>

#include <yy981/string.h>

#include "ui.h"
#include "text.h"
#include "../master.h"


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
}

std::map<std::vector<std::string>, std::function<void()>> commands = {
	{{"list","l","L"}, ui::list},
	{{"help","h","H"}, ui::help},
	{{"encrypt","enc","en","e"}, ui::encrypt},
	{{"decrypt","dec","de","d"}, ui::decrypt}
};

void UI() {
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
