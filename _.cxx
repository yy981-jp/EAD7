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