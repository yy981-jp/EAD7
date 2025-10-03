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

#include "def.h"
#include "base.h"
#include "master.h"
// #include "interface.h"
#include "CUI/ui.h"
#include "CUI/text.h"

std::vector<std::string> ca;

inline void init() {
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	if (!fs::exists(SD)) {
		fs::create_directories(SD);
		std::cout << t::banner << t::setup;
	}

	if (sodium_init() < 0) throw std::runtime_error("libsodium init failed\n");
	AESNI = sodium_runtime_has_aesni();
}

int main(int argc, char* argv[]) {
	ca = st::charV(argc,argv);
	QApplication app(argc, argv);
	init();
	// UI();
	gmain();
}
