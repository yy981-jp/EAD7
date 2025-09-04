#include <iostream>
#include <string>
#include <vector>
#include <fstream>
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


bool GUI = false;


inline void init() {
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	fs::create_directories(SDM);

	if (sodium_init() < 0) return_e("libsodium init failed\n");
	AESNI = sodium_runtime_has_aesni();
}

int main(int argc, char* argv[]) {
	init();
	
}
