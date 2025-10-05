#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <windows.h>

#include <cryptopp/cryptlib.h>
#include <sodium.h>

#include <QtWidgets/QApplication>

#include <yy981/string.h>
#include <yy981/proc.h>

#include "test.cpp"
#include "def.h"
#include "base.h"
#include "master.h"
// #include "interface.h"
#include "CUI/ui.h"
#include "CUI/text.h"

QApplication* app;
std::vector<std::string> ca;
UINT oldOutCP, oldInCP;

inline void init() {
	oldOutCP = GetConsoleOutputCP();
	oldInCP  = GetConsoleCP();
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	if (!fs::exists(SD)) {
		fs::create_directories(SD);
		std::cout << t::banner << t::setup;
	}

	if (sodium_init() < 0) throw std::runtime_error("libsodium init failed\n");
	AESNI = sodium_runtime_has_aesni();
}

inline void end() {
	SetConsoleOutputCP(oldOutCP);
	SetConsoleCP(oldInCP);
}

int main(int argc, char* argv[]) {
	app = new QApplication(argc, argv);
	ca = st::charV(argc,argv);
	init();
	int result = UI();
	end();
	return result;
}
