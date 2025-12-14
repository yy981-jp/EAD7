#pragma once
#include <QtWidgets/QMainWindow>

#include "def.h"

namespace Ui { class MainWindow; }

extern std::string prompt(const std::string& placeholderText);
extern json getKIDEntry(const std::string& label, const BIN& mk);

namespace u {
    void log(const std::string& str, bool continueLine = false);
    void stat(const std::string& str);
    void sl(const std::string& str);
}

void crashReport(const std::string& text);
