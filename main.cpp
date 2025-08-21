#include <iostream>
#include <string>
#include <vector>
#include <cryptopp/cryptlib.h>
#include <cryptopp/hkdf.h>
#include <cryptopp/sha.h>
#include <cryptopp/secblock.h>

#include <yy981/string.h>

using namespace CryptoPP;

#include "base.h"
#include "master.h"


int main(int argc, char* argv[]) {
	std::vector<std::string> input = st::charV(argc,argv);

	// for (const KIDList& e: parseKIDList("../test.kid.m.txt")) std::cout << e.dat << "|" << e.ex << "\n";
	createMK("correct horse battery staple");
	return 100;
	if (argc==2 || is_or(input[1],"manage","master")) {
		mmain();
		return 981;
	}

}
