#include <string>
#include <map>
#include <bitset>

#include <yy981/random.h>

#include "EAD7_db.h"
#include "EAD7_def.h"


namespace enc {

	EADdat main(std::string plain) {
		// 毎文字分離
		char lead_byte;
		std::string encoded;
		size_t utf8_len(0), input_size(0);
		for (size_t pos = 0; pos < plain.size(); pos += utf8_len) {
			++input_size;
			lead_byte = plain[pos];
			if (lead_byte < 0x80) utf8_len=1; else if (lead_byte < 0xE0) utf8_len=2; else if (lead_byte < 0xF0) utf8_len=3; else utf8_len=4; // UTF8タイプ判定
			std::string i_char=plain.substr(pos, utf8_len);
			if (db::enc.contains(i_char)) encoded += db::enc[i_char]; // データベースに存在
				else encoded += std::to_string(randomNum(0,9)) + i_char + std::to_string(randomNum(0,9)); // データベースに存在しない
		}
		// 付属情報計算
		// genInfoEnc(/*input_size*/);
	}
	
	
}