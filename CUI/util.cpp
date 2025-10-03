#include <iostream>
#include <windows.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <conio.h>
#include "ui.h"
#include "../master.h"


namespace env {
	UINT cp;
}

void clearPreviousConsoleLine(const bool currentLine = false) {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) return;

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;

	SHORT targetY;
	if (!currentLine) targetY = csbi.dwCursorPosition.Y - 1;
		else targetY = csbi.dwCursorPosition.Y;
	if (targetY < 0) return;

	COORD startCoord;
	startCoord.X = 0;
	startCoord.Y = targetY;

	DWORD consoleWidth = csbi.dwSize.X;
	DWORD written = 0;

	FillConsoleOutputCharacterA(hOut, ' ', consoleWidth, startCoord, &written);
	FillConsoleOutputAttribute(hOut, csbi.wAttributes, consoleWidth, startCoord, &written);
	SetConsoleCursorPosition(hOut, startCoord);
}

std::string inp(const std::string& out) {
	std::cout << out;
	std::string in;
	std::cin >> in;
	return in;
}

std::string inp_s(const std::string& out) {
	const std::string r = inp(out);
	clearPreviousConsoleLine();
	std::cout << out + "*****\n";
	return r;
}

void out_s(const std::string& out) {
	std::cout << "spaceかenterを押して続行): "<< out;
	while (true) {
		int key = _getch(); // 1文字取得（表示しない）
		if (key == ' ' || key == '\r') { // ' ' はスペース, '\r' はEnter
			break;
		}
	}
	clearPreviousConsoleLine(true);
	std::cout << "***************\n";
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
		json j = json::parse(input);
	}
	catch (const json::parse_error&) {
		return false;
	}
	return true;
}

FDat getJsonType(const std::string& file) {
	json j = json::parse(file);
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
	if (file.stem()=="MK") return FDat(FSType::MK,readJson(file.string()));
	if (file.stem().extension()==".kid") return FDat(FSType::kid,readJson(file.string()));
	if (file.stem().extension()==".kek") return getJsonType(file.string());
	return FDat(FSType::unknown_file,{});
}

FDat getFileType(const std::string& file) {
	if (fs::exists(file)) return getFileType(fs::path(file));
	if (isJson(file)) return getJsonType(file);
	if (isBase64UrlSafe(file)) {
		if (base::dec64(file)[0]==HEADER::magicData) return FDat(FSType::encBin,{});
		else return FDat(FSType::unknown_bin,{});
	}
	return FDat(FSType::unknown,{});
}
/*
void loadSubcommand(const std::map<std::vector<std::string>, std::function<void()>> commandList) {
	for (auto [aliases,handler]: commandList) {
		if (is_or(ca[1],aliases)) {
			handler();
			return;
		}
	}
	throw std::runtime_error("CLI引数エラー argc2");
}
*/