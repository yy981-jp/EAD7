#pragma once
#include <cryptopp/secblock.h>
#include <nlohmann/json.hpp>

using namespace CryptoPP;
using BIN = SecByteBlock;
namespace fs = std::filesystem;

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

extern const std::string SD;
extern const std::string SDM;
extern bool AESNI;

namespace path {
	extern const std::string MK,p_kek,cus_kek;
}