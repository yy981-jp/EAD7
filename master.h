#pragma once
#include <string>
#include <vector>
#include <cryptopp/secblock.h>

using namespace CryptoPP;


struct KIDList {
	std::string dat,ex;
};

void mmain();

std::vector<KIDList> parseKIDList(const std::string& fn, const SecByteBlock &hmacKey);
