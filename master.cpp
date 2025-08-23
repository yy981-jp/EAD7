#include "master.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <fstream>

#include <cryptopp/cryptlib.h>
#include <cryptopp/sha.h>
// #include <cryptopp/hmac.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>


#include <yy981/proc.h>

#include "def.h"

void manageMK() {}
void manageKEK() {
	std::cout << "対象ファイルパスを入力: ";
	std::string path;
	std::cin >> path;
	std::cout << "編集の終了を待機中";
	proc::start(path,"",true);
	// std::vector<KIDList> list = parseKIDList(path);
	// recal
	// INS1
}


void mmain() {
	std::cout << "[EAD7管理画面]\n1. MK管理\n2. KEK(KID)管理\n3. MK生成(非常時)\n4. KEK生成";
	std::string in;
	std::cin >> in;
	int i = std::stoi(in);
	switch (i) {
		case 1: manageMK(); break;
		case 2: manageKEK(); break;
		// case 3: createMK(); break;
		// case 4: createKEK(); break;
	}
	std::cout << "\n\n\n";
}
