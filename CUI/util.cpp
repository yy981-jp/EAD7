#include <iostream>
#include <windows.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include "ui.h"
#include "../master.h"


std::string inp(const std::string& out) {
	std::cout << out;
	std::string in;
	std::cin >> in;
	return in;
}

char choice(const std::string& message, const std::string& validChars) {
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;

	// 入力モードを保存
	GetConsoleMode(hStdin, &mode);
	SetConsoleMode(hStdin, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));

	char result = '\0';
	INPUT_RECORD record;
	DWORD read;

	std::cout << message << " [" << validChars << "]? ";

	bool done = false;
	while (!done) {
		ReadConsoleInput(hStdin, &record, 1, &read);

		if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
			char ch = record.Event.KeyEvent.uChar.AsciiChar;
			if (ch != 0) {
				char lowerCh = std::tolower(static_cast<unsigned char>(ch));

				for (char vc : validChars) {
					if (lowerCh == std::tolower(static_cast<unsigned char>(vc))) {
						result = vc; // 戻り値は validChars に合わせる
						std::cout << vc << "\n";
						done = true;
						break; // 内側の for を抜ける
					}
				}
			}
		}
	}

	// 入力モードを元に戻す
	SetConsoleMode(hStdin, mode);
	return result;
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
	try {
		auto j = nlohmann::json::parse(input);
	}
	catch (const nlohmann::json::parse_error&) {
		return false;
	}
	return true;
}

FDat getJsonType(const std::string& file) {
	json j = readJson(file);
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
	if (file.stem()=="MK") return FDat(FSType::MK,readJson(file.string()));
	if (file.stem().extension()==".kid") return FDat(FSType::kid,readJson(file.string()));
	if (file.stem().extension()==".kek") return getJsonType(file.string());
	return FDat(FSType::unknown_file,{});
}

FDat getFileType(const std::string& file) {
	if (fs::exists(file)) return getFileType(fs::path(file));
	if (isBase64UrlSafe(file)) {
		if (base::dec64(file)[0]==HEADER::magicData) return FDat(FSType::encBin,{});
		else return FDat(FSType::unknown_bin,{});
	}
	if (isJson(file)) return getJsonType(file);
	return FDat(FSType::unknown,{});
}