#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include <yy981/proc.h>

#include "master.h"

void mmain() {}

/*
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
*/