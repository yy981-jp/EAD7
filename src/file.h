#pragma once
#include <array>

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
