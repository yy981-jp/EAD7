#pragma once
#include <vector>
#include <string>
#include <windows.h>
#include "../GUI/ui.h"
#include "../def.h"
#include "../base.h"

extern std::vector<std::string> ca;
extern void UI();
extern std::string inp(const std::string& out);
extern std::string inp_s(const std::string& out);
extern void out_s(const std::string& out);
extern char choice(const std::string& message, const std::string& validChars);

namespace env {
	extern UINT cp;
}

enum class FSType {
	MK, kid, p_kek, raw_kek, cus_kek, adm_kek, dst_kek,
	base64, encBin, json,
	fe_e7, bin_e7, unknown, unknown_base64, unknown_bin, unknown_json, unknown_file
};
enum class stringType {
	path, b64, json
};

struct FDat {
	FSType type;
	json json;
};

extern FDat getFileType(std::string& file);
extern FDat getFileType(const fs::path& file);

using KIDIndex = std::map<std::string,std::string>;
enum class KIDIndexType {
	label
};

extern KIDIndex createKIDIndex(const json& j, KIDIndexType t = KIDIndexType::label);


inline std::string getAdmKEKPath(const std::string& name) {
	return SDMK + name + ".adm.kek.e7";
}

inline std::string convUnixTime(const int64_t& t) {
	std::ostringstream oss;
	oss << std::put_time(std::localtime(&t), "%Y/%m/%d-%H:%M:%S");
	return oss.str();
}

inline void openFile(const std::string& path) {
	ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

inline uint8_t cmkid(const std::string& mkid_s) {
	uint8_t mkid = std::stoi(mkid_s);
	if (!(mkid>=0 || mkid<=255)) {throw std::runtime_error("MKIDは0~255である必要があります");}
	return mkid;
}
