		for (KIDList e: list) std::cout << e.dat << " | " << e.ex << "\n";
		std::cout << "DEL,ADD,EDI,RCL,EXIT): ";
		std::string in;
		std::cin >> in;
		if (is_or(in,"d","del","DEL")) {
			std::cout << "対象のIDを入力: "
			list.erase()
		} else if (is_or(in,"a","add","ADD")) {
			
		} else if (is_or(in,"e","edi","EDI")) {
			
		} else if (is_or(in,"r","rcl","RCL","recal")) {
			
		} else if (is_or(in,"x","exit","EXIT")) {
			
		}



{
    // MK (32Bランダム例)
    SecByteBlock MK(32);
    for (size_t i = 0; i < MK.size(); ++i) MK[i] = i; // サンプル値
	std::cout << "MK:  " << base::enHex(MK) << "\n";

    // KIDやNonce
    std::string KID = "KID-EXAMPLE-1234"; // 例
    std::string Nonce = "RANDOM-NONCE12";  // 例12バイト

    // 1️⃣ KEK生成
    std::string infoKEK = "EAD7|KEK|" + KID;
    SecByteBlock KEK = deriveKey(MK, infoKEK, 32);
    std::cout << "KEK: " << base::enHex(KEK) << std::endl;

    // 2️⃣ DEK生成
    std::string infoDEK = "EAD7|DEK|" + Nonce;
    SecByteBlock DEK = deriveKey(KEK, infoDEK, 32);
    std::cout << "DEK: " << base::enHex(DEK) << std::endl;

    return 0;
}



	std::cout << "インデックスに既にMKが存在します\nMKが消えるとそこから派生したKEKなどはすべて再生成できなくなります 新しい鍵の派生も出来ません\n"
			  << "その危険性を理解したうえでこの処理を実行しますか? この操作は通常、誤って生成された鍵など不要なMKのみに行うべきものです\n本当に実行する場合は、現在の日時を入力してください (例:23日 -> 23):";






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
