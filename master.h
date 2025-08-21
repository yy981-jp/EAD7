#pragma once
#include <string>
#include <vector>
#include <cryptopp/secblock.h>

using namespace CryptoPP;


struct KIDList {
	std::string dat,ex;
};

struct MKEntryB64 {
	std::string salt,nonce,ct;
	bool status = true;
	MKEntryB64() {}
	MKEntryB64(bool status): status(status) {}
	operator bool() {return status;}
};

extern MKEntryB64 createMK(const std::string& pass);
extern std::vector<unsigned char> readMK(const std::string& pass, const MKEntryB64& res);


void mmain();

std::vector<KIDList> parseKIDList(const std::string& fn, const SecByteBlock &hmacKey);
