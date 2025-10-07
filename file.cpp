#include "master.h"


class ChunkReader {
	size_t chunkSize;
	std::ifstream file;
	BIN buffer;
public:
	binFReader(const std::string& filename, const short& chunkSizeMB) {
		file(filename, std::ios::binary | std::ios::ate);
		chunkSize = chunkSizeMB * 1024 * 1024;
		buffer(chunkSize);
		fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
	}

	size_t readNextChunk() {
		file.read(buffer.data(), chunkSize);
		return file.gcount();  // 実際に読み込んだバイト数
	}

	const char* data() const { return buffer.data(); }
	bool eof() const { return file.eof(); }
	BIN hmac(32);
	size_t fileSize;
};


namespace EAD7 {
	void encFile(const BIN& kek, const BIN& plaintext, const uint8_t& mkid_i, const BIN& kid, const std::string& path, const size_t& chunkSizeMB) {
		
	}
}