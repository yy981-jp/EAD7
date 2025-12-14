#include <fstream>

#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include "master.h"
#include "base.h"
#include <limits>

#include  <iostream>
#pragma pack(push, 1)
struct fixedFHeader {
	uint64_t magic;
	uint8_t ver;
};
struct FHeader {
	uint8_t mkid;
	std::array<uint8_t,16> kid;
	uint32_t chunkSize;
	uint64_t chunkNumber;
	uint32_t lastChunkSize;
};
struct FChunk {
	std::array<uint8_t,12> nonce;
	std::array<uint8_t,16> tag;
};
#pragma pack(pop)
static_assert(sizeof(fixedFHeader) == 1+8, "fixedFHeader size mismatch!");
static_assert(sizeof(FHeader) == 1+16+8+8, "FHeader size mismatch!");
static_assert(sizeof(FChunk) == 12+16, "FChunk size mismatch!");


static size_t toBufferSize(uint64_t requested) {
	// clamp to allowed range
	uint64_t v = requested;
	if (v < CHUNKSIZE::min) v = CHUNKSIZE::min;
	if (v > CHUNKSIZE::max) v = CHUNKSIZE::max;
	// align to 16-byte boundary for AES-GCM block friendliness
	// if (v % 16 != 0) v += (16 - (v % 16));
	if (v > static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
		throw std::runtime_error("chunkSize too large for this platform");
	return static_cast<size_t>(v);
}

class binFReader {
protected:
	std::ifstream file;
public:
	binFReader(const std::string& path) {
		file.open(fs::path(to_wstring(path)), std::ios::binary | std::ios::ate);
		if (!file) throw std::runtime_error("binFReader::file open error");
		data.resize(loadChunkSize);
		fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
	}
	binFReader(const std::string& path, const uint64_t requestedChunkSize): loadChunkSize(toBufferSize(requestedChunkSize)){
		file.open(fs::path(to_wstring(path)), std::ios::binary | std::ios::ate);
		if (!file) throw std::runtime_error("binFReader::file open error");
		data.resize(loadChunkSize);
		fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
	}

	void init(const uint64_t requestedChunkSize) {
		loadChunkSize = toBufferSize(requestedChunkSize);
		// ensure internal buffer matches requested chunk size
		data.resize(loadChunkSize);
	}

	std::streampos next(uint32_t readChunkSize = 0) {
		if (readChunkSize == 0) readChunkSize = loadChunkSize;
			else data.resize(readChunkSize);
		++currentChunk;
		file.read(reinterpret_cast<char*>(data.data()), readChunkSize);
		return file.gcount();  // 実際に読み込んだバイト数
	}

	operator bool() {
		if (!file.is_open()) return false;
		if (!file.good()) return false;
		std::streampos pos = file.tellg();
		if (pos == std::streampos(-1)) return file.good(); // tellg failed -> fall back to stream state
		return static_cast<size_t>(pos) < fileSize;
	}

	BIN data;
	size_t fileSize;
	uint64_t currentChunk = 0;
	size_t loadChunkSize = 0;
};

class E7BinFReader: public binFReader {
public:
	E7BinFReader(const std::string& path): binFReader(path) {
		if (fileSize < sizeof(FHeader)+32) throw std::runtime_error("E7BinFReader::ファイルサイズ不足");
		file.seekg(-32, std::ios::end);
		file.read(reinterpret_cast<char*>(hmac.data()), hmac.size());

		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(&fh), sizeof(fixedFHeader));

		if (fh.magic != FHEADER::magicData || fh.ver != FHEADER::verData) throw std::runtime_error("e7バイナリファイルが不正な内容です");

		file.read(reinterpret_cast<char*>(&h), sizeof(FHeader));
		
		totalChunk = h.chunkNumber;
		init(h.chunkSize+sizeof(FChunk));
	}

	std::streampos next() {
		if (eof) return 0;
		// data load
		std::streampos readBytes;
		if (currentChunk+1 >= totalChunk) {
			eof = true;
			readBytes = binFReader::next(h.lastChunkSize + sizeof(FChunk));
		} else readBytes = binFReader::next();
		// std::cout << "D: E7BinFReader::next() readBytes: " << readBytes << " currentChunk: " << currentChunk << "/" << totalChunk << "\n";

		// chunk fixed load
		FChunk chunk_header;
		// 確実に実際に読み込んだバイト数を使う（最後のチャンクは loadChunkSize より小さい可能性がある）
		if (readBytes < static_cast<std::streampos>(sizeof(FChunk))) throw std::runtime_error("E7BinFReader::next()::chunk too small");
		
		memcpy(&chunk_header, data.data(), sizeof(FChunk));
		
		// ct を実際のペイロード長で確保してからコピー
		// 最後のチャンク時は h.lastChunkSize を、通常時は h.chunkSize を使用
		uint32_t payloadSize = (eof ? h.lastChunkSize : h.chunkSize);
		if (payloadSize == 0) payloadSize = static_cast<size_t>(readBytes) - sizeof(FChunk);
		ct.resize(payloadSize);
		memcpy(ct.data(), data.data() + sizeof(FChunk), payloadSize);

		// set
		nonce = conv::ARRtoBIN<12>(chunk_header.nonce);
		tag = conv::ARRtoBIN<16>(chunk_header.tag);

		return file.gcount();
	}

	operator bool() {
		if (eof) return false;
		if (!file.is_open()) return false;
		if (!file.good()) return false;
		std::streampos pos = file.tellg();
		if (pos == std::streampos(-1)) return file.good(); // tellg failed -> fall back to stream state
		return static_cast<size_t>(pos) < fileSize - 32; // footer分を除外
	}
	
	fixedFHeader fh;
	FHeader h;

	uint64_t totalChunk = 0;
	
	BIN hmac{32};

	bool eof = false;
	BIN nonce{12}, tag{16}, ct; // next()毎に更新
};

inline BIN deriveE7FileHmacKey(const BIN& key) {
	return deriveKey(key, "EAD7|E7File-HMAC", 32);
}

inline BIN deriveDECFileKey(const BIN& kek, const BIN& nonce) {
	std::string info = "EAD7|DECFile|v1";
	return deriveKey(kek, info, 32, nonce);
}


namespace EAD7 {
	void encFile(const BIN& kek, const std::string& path, const uint8_t& mkid, const BIN& kid, uint32_t chunkSize) {
		std::string opath = path + ".e7";
		std::ofstream ofs(fs::path(to_wstring(opath)),std::ios::binary);
		if (!ofs) throw std::runtime_error("encFile()::ofs");
		
		binFReader file(path,chunkSize);
		fixedFHeader fh(FHEADER::magicData,FHEADER::verData);
		uint32_t surplus = file.fileSize % file.loadChunkSize;
		uint64_t chunkNumber = (file.fileSize / file.loadChunkSize) + (surplus == 0 ? 0 : 1);
		FHeader h(mkid,conv::BINtoARR<16>(kid),chunkSize,chunkNumber,surplus);
		
		BIN hmacKey = deriveE7FileHmacKey(kek);
		HMAC<SHA256> hmac(hmacKey, hmacKey.size());

		hmac.Update(reinterpret_cast<const uint8_t*>(&fh), sizeof(fixedFHeader));
		hmac.Update(reinterpret_cast<const uint8_t*>(&h), sizeof(FHeader));
		ofs.write(reinterpret_cast<const char*>(&fh), sizeof(fixedFHeader));
		ofs.write(reinterpret_cast<const char*>(&h), sizeof(FHeader));
		
		// body構成
		while (file) {
			std::streampos readChunkSize = file.next();
			if (readChunkSize == 0) throw std::runtime_error("encFile()::チャンク読み込み失敗");
			BIN nonce = randomBIN(12);
			BIN decKey = deriveDECFileKey(kek, nonce);
			
			// 実際に読み込んだバイト数分だけ暗号化（最後のチャンクは小さい可能性がある）
			BIN toEncrypt(file.data.data(), static_cast<size_t>(readChunkSize));
			CryptoGCM out = encAES256GCM(decKey, nonce, toEncrypt);

/*			std::cout << "D: enc:nonce: " << base::encHex(nonce)
					  << " tag: " << base::encHex(out.tag)
					  << " ct size: " << out.cipher.size() << "\n";
*/

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

	/// @return errorChunkNumbers
	std::vector<uint64_t> decFile(const BIN& kek, const std::string& path) {
		std::vector<uint64_t> errorChunkNumbers;
		fs::path path_f(path);
		std::string opath = (path_f.parent_path()/path_f.stem()).string();
		std::ofstream ofs(fs::path(to_wstring(opath)),std::ios::binary);
		if (!ofs) std::runtime_error("decFile()::ofs");
		
		E7BinFReader file(path);
		
		BIN hmacKey = deriveE7FileHmacKey(kek);
		HMAC<SHA256> hmac(hmacKey, hmacKey.size());

		hmac.Update(reinterpret_cast<const uint8_t*>(&file.fh), sizeof(fixedFHeader));
		hmac.Update(reinterpret_cast<const uint8_t*>(&file.h), sizeof(FHeader));
		
		// body構成
		while (file) {
			std::streampos readBytes = file.next();
			if (readBytes == 0) throw std::runtime_error("decFile()::チャンク読み込み失敗");
			BIN decKey = deriveDECFileKey(kek, file.nonce);
			
			BIN out;

/*			std::cout << "D: nonce: " << base::encHex(file.nonce)
					  << " tag: " << base::encHex(file.tag)
					  << " ct size: " << file.ct.size() << "\n";
*/
			try {
				out = decAES256GCM(decKey, file.nonce, file.ct, file.tag);
			} catch (const CryptoPP::Exception& e) {
				out.resize(file.h.chunkSize); // ダミー出力 0埋め
				errorChunkNumbers.push_back(file.currentChunk);
				continue;
			}

			// Chunk: 実際に読み込んだバイト数分をHMAC更新
			hmac.Update(reinterpret_cast<const uint8_t*>(file.data.data()), static_cast<size_t>(readBytes));
			ofs.write(reinterpret_cast<const char*>(out.data()), out.size());
		}
		
		// 全体HMAC検証
		BIN digest(hmac.DigestSize());
		hmac.Final(digest);
		if (digest != file.hmac) errorChunkNumbers.push_back(0); // 全体HMACエラー

		return errorChunkNumbers;
	}
}