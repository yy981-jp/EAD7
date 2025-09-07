#include "text.h"


bool interactive = false;
bool UISwitch_failed = false;

int UI(std::vector<std::string> ca) {
	switch (ca.size()) {
		case 1: interactive = true; break;
		
		case 2:
			if (is_or(ca[1],"manage","master")) {
				mmain();
				return 981;
			} else if (is_or(ca[1],"list","l","L")) {
				
			} else if (is_or(ca[1]),"help","h","H") {
				std::cout << t::banner;
			} else UISwitch_failed = true;
		
		
		
		break; case 3:
			if (is_or(ca[1],"enc","en","e","E")) {}
			else if (is_or(ca[1],"dec","de","d","D")) {}
			else UISwitch_failed = true;
		
		
		
		break; default: UISwitch_failed = true; break;
	}
	
	if (UISwitch_failed) return_e("CLI引数エラー \"EAD7 h\"を参照してください");
	return 0;
}
