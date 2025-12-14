#pragma once
#include "def.h"

struct VER {
	constexpr VER(const uint16_t& major, const uint16_t& minor, const uint16_t& patch = 0): major(major), minor(minor), patch(patch) {}
	constexpr VER() {loadCSV();}
	
	uint16_t gen{7}, major{0}, minor{0}, patch{0};
	
	constexpr std::string str() const {
		return "v"+std::to_string(major)+"."+std::to_string(minor)+(patch==0? "": "."+patch);
	}
	
	constexpr uint64_t num() const {
		return (uint64_t)major<<(16*2) | (uint64_t)minor<<(16*1) | (uint64_t)patch;
	}
	constexpr uint64_t numALL() const {
		return (uint64_t)gen<<(16*3) | (uint64_t)major<<(16*2) | (uint64_t)minor<<(16*1) | (uint64_t)patch;
	}

	void loadCSV();
	constexpr static std::string csvPath = "ver.csv";
};

extern VER ver;