#include <iostream>
#include <string>
#include <vector>
#include <cryptopp/cryptlib.h>

#include <yy981/string.h>

#include "def.h"

#include "base.h"
#include "master.h"


int main(int argc, char* argv[]) {
	std::vector<std::string> input = st::charV(argc,argv);

	// for (const KIDList& e: parseKIDList("../test.kid.m.txt")) std::cout << e.dat << "|" << e.ex << "\n";
	std::cout << base::encHex(loadMK("MK.e7",2,"pass12345"));
	return 100;
	if (argc==2 || is_or(input[1],"manage","master")) {
		mmain();
		return 981;
	}

}
