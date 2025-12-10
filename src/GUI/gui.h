#pragma once
#include <QtWidgets/QMainWindow>

#include "def.h"

namespace Ui { class MainWindow; }

struct VER {
	constexpr VER(const uint16_t& major, const uint16_t& minor, const uint16_t& patch = 0): major(major), minor(minor), patch(patch) {}
	
	uint16_t gen{7}, major, minor, patch;
	
	constexpr std::string str() const {
		return "v"+std::to_string(major)+"."+std::to_string(minor)+(patch==0? "": "."+patch);
	}
	
	constexpr uint64_t num() const {
		return (uint64_t)major<<(16*2) | (uint64_t)minor<<(16*1) | (uint64_t)patch;
	}
	constexpr uint64_t numALL() const {
		return (uint64_t)gen<<(16*3) | (uint64_t)major<<(16*2) | (uint64_t)minor<<(16*1) | (uint64_t)patch;
	}
};
constexpr VER ver(0,3);

extern std::string prompt(const std::string& placeholderText);
extern json getKIDEntry(const std::string& label, const BIN& mk);

namespace u {
    void log(const std::string& str, bool continueLine = false);
    void stat(const std::string& str);
    void sl(const std::string& str);
}

void crashReport(const std::string& text);
