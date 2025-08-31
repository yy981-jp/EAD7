#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <windows.h>

#include <cryptopp/cryptlib.h>
#include <sodium.h>

#include <nlohmann/json.hpp>

#include <yy981/string.h>
#include <yy981/return.h>

#include "def.h"
#include "base.h"
#include "master.h"
#include "interface.h"


using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;


bool GUI = false;


int main(int argc, char* argv[]) {
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	BIN bin = base::dec64("mZ3v8Jp1N+4xQ6t8H2l9aYvK0sF3bQ7d9XwR1oPj2Zs=");
	std::cout << base::enc64(bin);

	fs::create_directories(SDM);
	std::vector<std::string> input = st::charV(argc,argv);

	BIN mk = loadMK(1,"testabc");
	createKID(mk,1,KIDEntry("testKey","試験用"));

	
	
/*
	switch (argc) {
		case 1: GUI = true; break;
		
		case 2:
			if (is_or(input[1],"manage","master")) {
				mmain();
				return 981;
			}
		
		
		
		break; case 3:
			if (is_or(input[1]),"enc","en","e","E") {}
			else if (is_or(input[1]),"dec","de","d","D") {}
		
		
		
		break; default: return_e("CLI引数エラー");
	}*/
}
