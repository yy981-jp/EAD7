#include <iostream>
#include <windows.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <conio.h>
#include "ui.h"
#include "master.h"


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
	std::cout << "space縺菊nter繧呈款縺励※邯夊｡・: "<< out;
	while (true) {
		int key = _getch(); // 1譁・ｭ怜叙蠕暦ｼ郁｡ｨ遉ｺ縺励↑縺・ｼ・
		if (key == ' ' || key == '\r') { // ' ' 縺ｯ繧ｹ繝壹・繧ｹ, '\r' 縺ｯEnter
			break;
		}
	}
	clearPreviousConsoleLine(true);
	std::cout << "***************\n";
}



char choice(const std::string& message, const std::string& validChars) {
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;

	// 蜈･蜉帙Δ繝ｼ繝峨ｒ菫晏ｭ・
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
						result = vc; // 謌ｻ繧雁､縺ｯ validChars 縺ｫ蜷医ｏ縺帙ｋ
						std::cout << vc << "\n";
						done = true;
						break; // 蜀・・縺ｮ for 繧呈栢縺代ｋ
					}
				}
			}
		}
	}

	// 蜈･蜉帙Δ繝ｼ繝峨ｒ蜈・↓謌ｻ縺・
	SetConsoleMode(hStdin, mode);
	return result;
}

std::string toUTF8(const std::string& s) {
	// CP932 竊・UTF-16
	int wlen = MultiByteToWideChar(env::cp, 0, s.c_str(), (int)s.size(), nullptr, 0);
	std::wstring wbuf(wlen, 0);
	MultiByteToWideChar(env::cp, 0, s.c_str(), (int)s.size(), &wbuf[0], wlen);

	// UTF-16 竊・UTF-8
	int len = WideCharToMultiByte(CP_UTF8, 0, wbuf.c_str(), (int)wbuf.size(), nullptr, 0, nullptr, nullptr);
	std::string out(len, 0);
	WideCharToMultiByte(CP_UTF8, 0, wbuf.c_str(), (int)wbuf.size(), &out[0], len, nullptr, nullptr);

	return out;
}

/*
void loadSubcommand(const std::map<std::vector<std::string>, std::function<void()>> commandList) {
	for (auto [aliases,handler]: commandList) {
		if (is_or(ca[1],aliases)) {
			handler();
			return;
		}
	}
	throw std::runtime_error("CLI蠑墓焚繧ｨ繝ｩ繝ｼ argc2");
}
*/
