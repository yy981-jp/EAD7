#include <iostream>
#include <map>
#include <functional>
#include <stdexcept>

#include <yy981/string.h>

#include "ui.h"
#include "text.h"


bool interactive = false;
bool UISwitch_failed = false;

namespace ui {
	void list() {
		
	}
	
	void help() {
		std::cout << t::banner << t::help;
	}
	
	
	
	void encrypt() {
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
