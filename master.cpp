#include "master.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <fstream>

#include <cryptopp/cryptlib.h>
#include <cryptopp/sha.h>
#include <cryptopp/hmac.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>


#include <yy981/proc.h>

#include "def.h"

void manageMK() {}
void manageKEK() {
	std::cout << "対象ファイルパスを入力: ";
	std::string path;
	std::cin >> path;
	std::cout << "編集の終了を待機中";
	proc::start(path,"",true);
	// std::vector<KIDList> list = parseKIDList(path);
	// recal
	// INS1
}


std::string calcHMAC(const std::string &data, const BIN &key) {
	std::string mac;
	HMAC<SHA256> hmac(key, key.size());

	StringSource ss(
		data, true,
		new HashFilter(hmac,
			new Base64Encoder(
				new StringSink(mac), false // false = 改行なし
			)
		)
	);

	return mac;
}

std::vector<KIDList> parseKIDList(const std::string& fn, const BIN &hmacKey) {
	std::ifstream f(fn);
	if (!f) throw std::runtime_error("File open failed");

	std::string firstLine;
	if (!std::getline(f, firstLine)) throw std::runtime_error("Empty file");

	// 本文全体を読み込み
	std::string body, line;
	while (std::getline(f, line)) {
		body += line + "\n"; // 改行も含めてHMAC対象
	}

	// HMAC検証
	std::string expected = calcHMAC(body, hmacKey);
	if (expected != firstLine) {
		throw std::runtime_error("HMAC verification failed");
	}

	// パース
	std::vector<KIDList> result;
	std::istringstream iss(body);
	while (std::getline(iss, line)) {
		if (line.size() < 24) continue; // 不正行はスキップ
		result.push_back({ line.substr(0, 24), line.substr(25) });
	}
	return result;
}

void mmain() {
	std::cout << "[EAD7管理画面]\n1. MK管理\n2. KEK(KID)管理\n3. MK生成(非常時)\n4. KEK生成";
	std::string in;
	std::cin >> in;
	int i = std::stoi(in);
	switch (i) {
		case 1: manageMK(); break;
		case 2: manageKEK(); break;
		// case 3: createMK(); break;
		// case 4: createKEK(); break;
	}
	std::cout << "\n\n\n";
}
