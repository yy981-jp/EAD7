#pragma once

#include <string>
#include <cstdint>
#include <iomanip>

#include <windows.h>

#include "../master.h"
#include "../base.h"

std::string formatBytes(uint64_t bytes);
std::string formatSeconds(uint64_t totalSeconds);

inline void openFile(const std::string& path) {
	ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
inline std::string convUnixTime(const int64_t& t) {
	std::ostringstream oss;
	oss << std::put_time(std::localtime(&t), "%Y/%m/%d-%H:%M:%S");
	return oss.str();
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


bool isBase64UrlSafe(const std::string& input);
bool isJson(const std::string& input);
FDat getJsonType(const json& j);
FDat getFileType(const fs::path& file);
FDat getFileType(std::string& file);
