#pragma once
#include <string>
#include <unordered_map>
#include <bitset>


namespace map {
	namespace index {
		extern std::unordered_map<std::string, int> to_number;
		extern std::unordered_map<int, std::string> to_alphabet;
	}
	namespace def {
		extern std::unordered_map<std::string, int> to_number;
		extern std::unordered_map<int, std::string> to_alphabet;
	}
}



struct EADdat {
	EADdat(std::string data, std::bitset<6> error = std::bitset<6>());
	operator bool();
	std::string code();
	std::string data;
	std::bitset<6> error;
};


struct KEY {
	uint8_t version_major : 4;  // 上位バージョン（0〜15）
	uint8_t version_minor : 4;  // 下位バージョン（0〜15）
	uint32_t room_id : 32; // ID
	uint8_t check : 8; // チェックサム
};