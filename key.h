#pragma once
#include <string>
#include <cstdint>
#include <stdexcept>


struct KEY {
	uint8_t version_major : 4;
	uint8_t version_minor : 4;
	uint32_t id; // 32bit
	uint8_t check;
};


namespace key {
	uint64_t base52_decode(const std::string& str);
	KEY decode(const std::string& keystr);
	
	std::string base52_encode(uint64_t value);
	std::string encode(const KEY& key);
	
	uint8_t calc_check(const KEY& key);
}