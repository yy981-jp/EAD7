#include "util.h"

#include <iostream>
#include <string>
#include <array>
#include <sstream>
#include <iomanip>


std::string formatBytes(uint64_t bytes) {
	static constexpr std::array<std::string, 7> units = {
		"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"
	};

	double v = static_cast<double>(bytes);
	std::size_t i = 0;

	while (v >= 1024.0 && i + 1 < units.size()) {
		v /= 1024.0;
		++i;
	}

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(i == 0 ? 0 : 2)
		<< v << " " << units[i];
	return oss.str();
}

std::string formatSeconds(uint64_t totalSeconds) {
	uint64_t sec = totalSeconds;

	uint64_t days = sec / 86400;
	sec %= 86400;

	uint64_t hours = sec / 3600;
	sec %= 3600;

	uint64_t minutes = sec / 60;
	sec %= 60;

	std::ostringstream oss;
	bool first = true;

	if (days > 0) {
		oss << days << "d";
		first = false;
	}
	if (hours > 0 || !first) {
		if (!first) oss << ":";
		oss << hours << "h";
		first = false;
	}
	if (minutes > 0 || !first) {
		if (!first) oss << ":";
		oss << minutes << "m";
		first = false;
	}
	if (!first) oss << ":";
	oss << sec << "s";

	return oss.str();
}


bool isBase64UrlSafe(const std::string& input) {
	try {
		base::dec64(input);
	} catch (const Exception&) {
		return false;
	}

	return true;
}

bool isJson(const std::string& input) {
	// if (input.starts_with("{")) return false;
	try {
		json j = json::parse(input);
	}
	catch (...) {
		return false;
	}
	return true;
}

FDat getJsonType(const json& j) {
	if (j.contains("hmac")) return FDat(FSType::kid,j);
	std::string kek_type = j.at("type");
	if (kek_type=="p")   return FDat(FSType::p_kek,j);
	if (kek_type=="raw") return FDat(FSType::raw_kek,j);
	if (kek_type=="cus") return FDat(FSType::cus_kek,j);
	if (kek_type=="adm") return FDat(FSType::adm_kek,j);
	if (kek_type=="dst") return FDat(FSType::dst_kek,j);
	return FDat(FSType::unknown_json,{});
}

FDat getFileType(const fs::path& file) {
	if (file.extension() != ".e7") return FDat(FSType::fe_e7,{});
	try {
		FHeader fh = getFileHeader(file.string());
		json j;
		j["chunkNumber"] = fh.chunkNumber;
		j["chunkSize"] = fh.chunkSize;
		j["lastChunkSize"] = fh.lastChunkSize;
		j["mkid"] = fh.mkid;
		j["kid"] = base::enc64(conv::ARRtoBIN(fh.kid));
		return FDat(FSType::bin_e7,j);
	} catch (...) {}
	if (file.stem()=="mk") return FDat(FSType::MK,readJson(file.string()));
	if (!file.stem().has_extension() && file.stem()=="kek") return FDat(FSType::p_kek,readJson(file.string()));
	if (file.stem().extension()==".kid") return FDat(FSType::kid,readJson(file.string()));
	if (file.stem().extension()==".kek") return getJsonType(readJson(file.string()));
	return FDat(FSType::unknown_file,{});
}

FDat getFileType(std::string& file) {
	while (file.starts_with(" ")) {file.erase(0,1);}
	if (fs::exists(to_wstring(file))) return getFileType(fs::path(file));
	if (isJson(file)) return getJsonType(readJson(file));
	if (isBase64UrlSafe(file)) {
		if (base::dec64(file)[0]==HEADER::magicData) return FDat(FSType::encBin,{});
		else return FDat(FSType::unknown_bin,{});
	}
	return FDat(FSType::unknown,{});
}
