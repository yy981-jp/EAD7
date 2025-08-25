#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include <cryptopp/cryptlib.h>
#include <sodium.h>

#include <yy981/string.h>
#include <yy981/return.h>

#include "def.h"
#include "base.h"
#include "master.h"

bool GUI = false;


int main(int argc, char* argv[]) {
	
	
	
	return 100;
	fs::create_directory(sd);
	std::vector<std::string> input = st::charV(argc,argv);
	std::cout << sd;

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
