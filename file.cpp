#include <fstream>

#include "master.h"


struct FHeader {};

class binFReader {
protected:
	size_t chunkSize;
	std::ifstream file;
	BIN data;
public:
	binFReader(const std::string& path, const short& chunkSizeMB) {
		file.open(path, std::ios::binary | std::ios::ate);
		chunkSize = chunkSizeMB * 1024 * 1024;
		data.resize(chunkSize);
		fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
	}

	size_t next() {
		file.read(reinterpret_cast<char*>(data.BytePtr()), chunkSize);
		return file.gcount();  // 実際に読み込んだバイト数
	}

	bool eof() const { return file.eof(); }
	size_t fileSize;
};

class E7BinFReader: public binFReader {
public:
	E7BinFReader(const std::string& path, const short& chunkSizeMB): binFReader(path,chunkSizeMB) {
		if (fileSize < sizeof(FHeader)+32) throw std::runtime_error("E7BinFReader::ファイルサイズ不足");
		file.seekg(-32, std::ios::end);
		file.read(reinterpret_cast<char*>(hmac.BytePtr()), hmac.size());
		
	}
	
	BIN hmac{32};
};


namespace EAD7 {
	void encFile(const BIN& kek, const BIN& plaintext, const uint8_t& mkid_i, const BIN& kid, const std::string& path, const size_t& chunkSizeMB) {
		
	}
}