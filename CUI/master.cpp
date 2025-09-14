#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include <yy981/proc.h>

#include "../master.h"
#include "../base.h"
#include "ui.h"
#include "../GUI/ui.h"


json loadKIDEntry() { //mkid 1つずつのみ対応 増やしたかったらその時作る?
	int mkid = std::stoi(inp("対象KIDリストのMKID: "));
	if (!(mkid>=0 || mkid<=255)) {std::cerr << "MKIDは0~255である必要があります"; return {};}
	std::string pass = inp("MKIDのMKのパスワード: ");
	BIN mk = loadMK(mkid,pass);
	json kid = loadKID(mk,mkid);
	
}

namespace uim {
	void MK() {
		json MKw = readJson(path::MK);
		
		std::string index;
		std::cout << "存在するMKのID: ";
		for (auto [key,value]: MKw.items()) {
			std::cout << key << " ";
			index += key;
		}
		std::cout << "\n";
		switch (choice("操作内容 (終了:E,作成:C,追加:I,解読:R,ファイルを開く:O)","ECIRO")) {
			case 'E': return;
			case 'C': {
				int mkid = std::stoi(inp("新しいMKのインデックス: "));
				if (!(mkid>=0 || mkid<=255)) {std::cerr << "MKIDは0~255である必要があります"; return;}
				std::string pass = inp("新しいMKのパスワード: ");
				::createMK(mkid,pass);
				delm(pass);
			}
			case 'I': {
				int mkid = std::stoi(inp("追加するMKのインデックス: "));
				if (!(mkid>=0 || mkid<=255)) {std::cerr << "MKIDは0~255である必要があります"; return;}
				std::string pass = inp("追加するMKのパスワード: ");
				std::string mk_b64 = inp("追加するMK(base64): ");
				BIN mk = base::dec64(mk_b64);
				::createMK(mkid,pass,mk);
				delm(mk,mk_b64);
			} break; case 'R': {
				int mkid = choice("対象MKID(候補="+index+"): ",index) - '0';
				std::string pass = inp("対象MKのパスワード: ");
				BIN mk = loadMK(mkid,pass);
				std::string mk_b64 = base::enc64(mk);
				std::cout << "生MK(Base64): " << mk_b64 << "\n";
				delm(mk,mk_b64);
			} break; case 'O': proc::start(path::MK); break;
		}
	}
}

void mmain() {
	while (true) {
		char i = choice("[EAD7管理画面]\n1. MK管理\n2. KEK(KID)管理\n4. KEK生成", "");
		switch (i) {
			case 1: uim::MK(); break;
		}
		std::cout << "\n\n\n";
	}
}
