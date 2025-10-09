#include <fstream>

#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include "master.h"
#include "base.h"
#include "GUI/gui.h"


#pragma pack(push, 1)
struct fixedFHeader {
	uint8_t magic, ver;
};
struct FHeader {
	uint8_t mkid;
	std::array<uint8_t,16> kid;
	uint64_t chunkSize;
};
struct FChunk {
	std::array<uint8_t,12> nonce;
	std::array<uint8_t,16> tag;
};
struct footer {
	std::array<uint8_t,32> hmac;
};
#pragma pack(pop)
static_assert(sizeof(fixedFHeader) == 1+1, "fixedFHeader size mismatch!");
static_assert(sizeof(FHeader) == 1+16+8, "FHeader size mismatch!");
static_assert(sizeof(FHeader) == 1+16+8, "FHeader size mismatch!");

class binFReader {
protected:
	size_t chunkSize;
	std::ifstream file;
public:
	binFReader(const std::string& path, const uint64_t& chunkSize): chunkSize(chunkSize) {
		file.open(fs::path(to_wstring(path)), std::ios::binary | std::ios::ate);
		data.resize(chunkSize);
		fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
	}

	std::streampos next() {
		++totalChunk;
		file.read(reinterpret_cast<char*>(data.BytePtr()), chunkSize);
		return file.gcount();  // 実際に読み込んだバイト数
	}

	operator bool() const { return !file.eof(); }
	BIN data;
	size_t fileSize;
	int totalChunk = 0;
};

class E7BinFReader: public binFReader {
public:
	E7BinFReader(const std::string& path, const uint64_t& chunkSize): binFReader(path,chunkSize) {
		if (fileSize < sizeof(FHeader)+32) throw std::runtime_error("E7BinFReader::ファイルサイズ不足");
		file.seekg(-32, std::ios::end);
		
		file.read(reinterpret_cast<char*>(hmac.BytePtr()), hmac.size());
		file.seekg(0, std::ios::beg);
		
		file.read(reinterpret_cast<char*>(&fh), sizeof(fixedFHeader));
		if (fh.magic != HEADER::magicData || fh.ver != 1) throw std::runtime_error("e7バイナリファイルが不正な内容です");
	}
	
	fixedFHeader fh;
	FHeader h;
	
	BIN hmac{32};
};

inline BIN deriveE7FileHmacKey(const BIN& key) {
	return deriveKey(key, "EAD7|E7File-HMAC", 32);
}

inline BIN deriveDECFileKey(const BIN& kek, const BIN& nonce) {
	std::string info = "EAD7|DECFile|v1";
	return deriveKey(kek, info, 32, nonce);
}


BIN concat(const BIN& a, const BIN& b) {
    BIN out(a.size() + b.size());
    memcpy(out, a, a.size());
    memcpy(out + a.size(), b, b.size());
    return out;
}

namespace EAD7 {
	void encFile(const BIN& kek, const std::string& path, const uint8_t& mkid, const BIN& kid, const uint64_t& chunkSize) {
		std::string opath = path + ".e7";
		std::ofstream ofs(fs::path(to_wstring(opath)),std::ios::binary);
		if (!ofs) std::runtime_error("encFile()::ofs");
		
		binFReader file(path,chunkSize);
		fixedFHeader fh(HEADER::magicData,1);
		FHeader h(mkid,conv::BINtoARR<16>(kid),chunkSize);
		
		BIN hmacKey = deriveE7FileHmacKey(kek);
		HMAC<SHA256> hmac(hmacKey, hmacKey.size());

		hmac.Update(reinterpret_cast<const uint8_t*>(&fh), sizeof(fixedFHeader));
		hmac.Update(reinterpret_cast<const uint8_t*>(&h), sizeof(FHeader));
		ofs.write(reinterpret_cast<const char*>(&fh), sizeof(fixedFHeader));
		ofs.write(reinterpret_cast<const char*>(&h), sizeof(FHeader));
		
		// body構成
		while (file) {
			if (file.next() != chunkSize) std::runtime_error("encFile()::チャンク読み込み失敗");
			BIN nonce = randomBIN(12);
			BIN decKey = deriveDECFileKey(kek, nonce);
			
			CryptoGCM out = encAES256GCM(decKey, nonce, file.data);
			// Chunk構成
			BIN chunk(sizeof(FChunk)+out.cipher.size());
			FChunk chunk_fixed(conv::BINtoARR<12>(nonce),conv::BINtoARR<16>(out.tag));
			memcpy(chunk, &chunk_fixed, sizeof(FChunk));
			memcpy(chunk + sizeof(FChunk), out.cipher.data(), out.cipher.size());
			hmac.Update(reinterpret_cast<const uint8_t*>(chunk.data()), chunk.size());
			ofs.write(reinterpret_cast<const char*>(chunk.data()), chunk.size());
		}
		
		// HMAC書き込み
		BIN digest(hmac.DigestSize());
		hmac.Final(digest);
		ofs.write(reinterpret_cast<const char*>(digest.data()), digest.size());
	}
}