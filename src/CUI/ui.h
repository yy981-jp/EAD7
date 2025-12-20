#pragma once
#include <vector>
#include <string>
#include <windows.h>
#include "../GUI/cui.h"
#include "../def.h"
#include "../base.h"

extern std::vector<std::string> ca;
extern int UI();
extern std::string inp(const std::string& out);
extern std::string inp_s(const std::string& out);
extern void out_s(const std::string& out);
extern char choice(const std::string& message, const std::string& validChars);

namespace env {
	extern UINT cp;
}

struct EAD7ST {
	EAD7ST(const BIN& blob) {
		size_t pos = 0;
		magic = blob[pos++];
		ver = blob[pos++];
		mkid = blob[pos++];
		kid = BIN(blob.data() + pos, 16);  pos += 16;
		nonce = BIN(blob.data() + pos, 12); pos += 12;
		size_t cipher_size = blob.size() - pos - 16;
		cipher = BIN(blob.data() + pos, cipher_size);
		pos += cipher_size;
		tag = BIN(blob.data() + pos, 16);
		size = pos + 16;
	}
	byte magic, ver, mkid;
	BIN kid, nonce, cipher, tag;
	size_t size;
};


using KIDIndex = std::map<std::string,std::string>;
enum class KIDIndexType {
	label
};

KIDIndex createKIDIndex(const json& j, KIDIndexType t = KIDIndexType::label);

inline std::string getAdmKEKPath(const std::string& name) {
	return SDMK + name + ".adm.kek.e7";
}

inline uint8_t cmkid(const std::string& mkid_s) {
	uint8_t mkid = std::stoi(mkid_s);
	if (!(mkid>=0 || mkid<=255)) {throw std::runtime_error("MKIDは0~255である必要があります");}
	return mkid;
}
