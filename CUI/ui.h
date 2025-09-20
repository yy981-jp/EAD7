#pragma once
#include <vector>
#include <string>
#include "../GUI/ui.h"
#include "../def.h"
#include "../base.h"

extern std::vector<std::string> ca;
extern void UI();
extern std::string inp(const std::string& out);
extern char choice(const std::string& message, const std::string& validChars);

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

extern FDat getFileType(const std::string& file);

inline std::string convUnixTime(const int64_t& t) {
	std::ostringstream oss;
	oss << std::put_time(std::localtime(&t), "%Y/%m/%d-%H:%M:%S");
	return oss.str();
}
