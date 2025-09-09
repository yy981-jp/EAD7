#include <iostream>
#include <windows.h>
#include "ui.h"


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