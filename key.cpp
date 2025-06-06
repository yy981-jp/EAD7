#include "key.h"


namespace key {

	uint64_t base52_decode(const std::string& str) {
		if (str.size() != 10) throw std::invalid_argument("鍵の長さは10文字固定です");

		uint64_t value = 0;
		for (char c : str) {
			uint8_t index;
			if ('A' <= c && c <= 'Z') index = c - 'A';         // 0〜25
			else if ('a' <= c && c <= 'z') index = c - 'a' + 26; // 26〜51
			else throw std::invalid_argument("鍵に使えない文字が含まれています");

			value = value * 52 + index;
		}
		return value;
	}

	KEY decode(const std::string& keystr) {
		uint64_t raw = base52_decode(keystr);

		KEY key;
		key.check = raw & 0xFF;                  // 下位8bit
		key.id = (raw >> 8) & 0xFFFFFFFF;   // 次の32bit
		uint8_t ver = (raw >> 40) & 0xFF;        // 上位8bit
		key.version_major = ver >> 4;
		key.version_minor = ver & 0x0F;

		return key;
	}

	
	std::string base52_encode(uint64_t value) {
		const char base52_table[52] = {
			'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
			'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'
		};

		std::string result(10, 'A');
		for (int i = 9; i >= 0; --i) {
			result[i] = base52_table[value % 52];
			value /= 52;
		}
		return result;
	}

	std::string encode(const KEY& key) {
		uint64_t raw = 0;
		raw |= (uint64_t(key.version_major) << 52);
		raw |= (uint64_t(key.version_minor) << 48);
		raw |= (uint64_t(key.id) << 8);
		raw |= calc_check(key);

		return base52_encode(raw);
	}


	uint8_t calc_check(const KEY& key) {
		uint8_t v = (key.version_major << 4) | key.version_minor;

		uint8_t c = v;
		c ^= (key.id >> 24) & 0xFF;
		c ^= (key.id >> 16) & 0xFF;
		c ^= (key.id >> 8)  & 0xFF;
		c ^= (key.id)       & 0xFF;

		return c;
	}

}