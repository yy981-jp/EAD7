#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <windows.h>

#include <cryptopp/cryptlib.h>
#include <sodium.h>


#include <yy981/string.h>
#include <yy981/return.h>

#include "def.h"
#include "base.h"
#include "master.h"
#include "interface.h"


bool GUI = false;


inline void init() {
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	fs::create_directories(SDM);

	if (sodium_init() < 0) return_e("libsodium init failed\n");
	AESNI = sodium_runtime_has_aesni();
}

#include <conio.h>   // _getch()

void setCursorPos(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(hConsole, pos);
}

class TUI {
	HANDLE hConsole;
	CHAR_INFO* buffer;
	COORD bufSize;
	COORD bufCoord;
	SMALL_RECT readRegion;
	COORD originalCursor;
	
public:
	TUI() {
		DWORD count;
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		// コンソール情報取得
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hConsole, &csbi);
		SHORT width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		SHORT height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

		// 元の画面バッファを保存
		buffer = new CHAR_INFO[width * height];
		bufSize = { width, height };
		bufCoord = { 0, 0 };
		readRegion = { csbi.srWindow.Left, csbi.srWindow.Top,
								  csbi.srWindow.Right, csbi.srWindow.Bottom };
		ReadConsoleOutput(hConsole, buffer, bufSize, bufCoord, &readRegion);
		originalCursor = csbi.dwCursorPosition;
		
	    DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
		FillConsoleOutputCharacter(hConsole, ' ', cellCount, {0,0}, &count);
		FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, {0,0}, &count);
		SetConsoleCursorPosition(hConsole, {0,0});
	}
	
	~TUI() {
		// 元画面を復元
		WriteConsoleOutput(hConsole, buffer, bufSize, bufCoord, &readRegion);
		SetConsoleCursorPosition(hConsole, originalCursor);

		delete[] buffer;
	}
};

int choice(const std::vector<std::string> options) {
    int selected = 0;

    // 初回描画
    for (int i = 0; i < options.size(); ++i) {
        if (i == selected) std::cout << "> " << options[i] << "\n";
        else std::cout << "  " << options[i] << "\n";
    }

    while (true) {
        int ch = _getch();
        if (ch == 224) { // 矢印キーは2バイト
            ch = _getch();
            if (ch == 72) { // ↑
                selected = (selected - 1 + options.size()) % options.size();
            } else if (ch == 80) { // ↓
                selected = (selected + 1) % options.size();
            }
        } else if (ch == 13) { // Enter
            break;
        }

        // 再描画
        setCursorPos(0,0);
        for (int i = 0; i < options.size(); ++i) {
            if (i == selected) std::cout << "> " << options[i] << "  \n";
            else std::cout << "  " << options[i] << "  \n";
        }
    }

    return selected;
}

const std::string banner = "\
██████╗ █████╗ █████╗ ███████╗\n\
██╔═══╝██╔══██╗██╔═██╗╚═══██╔╝\n\
█████╗ ███████║██║ ██║   ██╔╝\n\
██╔══╝ ██╔══██║██║ ██║  ██╔╝\n\
██████╗██║  ██║█████╔╝ ██╔╝  (c) 2025 yy981\n\
╚═════╝╚═╝  ╚═╝╚════╝  ╚═╝\n";

int main() {
	init();

	TUI t;
	std::cout << banner;
	return choice({"c1","c2","c3","c4"});
}