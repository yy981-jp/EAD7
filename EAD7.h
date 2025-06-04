#include <EAD7_db.h>
#include <EAD7_def.h>


std::string room;
std::string genInfoEnc(size_t input_size) {
	
}



EADdat enc(std::string input) {
	// 毎文字分離
	size_t char_size=0, input_size=0;
	for (pos = 0; pos < input.size(); pos += char_size) {
		input_size++;
		lead = input[pos];
		if (lead < 0x80) char_size=1; else if (lead < 0xE0) char_size=2; else if (lead < 0xF0) char_size=3; else char_size=4;
		// UTF8タイプ判定
		std::string i_char=input.substr(pos, char_size);
		if (db::enc.find(i_char) != db::enc.end()) output += db::enc[i_char]; // データベースに存在
			else output += std::to_string(randomNum(0,9)) + i_char + std::to_string(randomNum(0,9)); // データベースに存在しない
	}
	// 付属情報計算
	genInfoEnc(input_size);
}

EADdat dec(std::string input) {
	
}
