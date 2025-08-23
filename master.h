#pragma once
#include <string>
#include <vector>

#include "def.h"



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

extern MKEntryB64 createMKCore(const std::string& pass);
extern BIN readMKCore(const std::string& pass, const MKEntryB64& res);

extern void createMK(const std::string& path, int index, const std::string& pass);
extern BIN readMK(const std::string& path, int index, const std::string& pass);

extern BIN createKEK(const std::string& KID);


void mmain();

std::vector<KIDList> parseKIDList(const std::string& fn, const BIN &hmacKey);
