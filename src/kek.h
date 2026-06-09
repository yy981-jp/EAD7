#pragma once
#include <string>
#include <sodium.h>
#include "crypto.h"

BIN deriveKEK(const BIN &mk, const std::string kid_b64);
json createRawKEK(const BIN &mk, json kek_json, const json &kid_json, const uint8_t &mkid);
json encAdmKEK(const BIN &mk, const json &raw_json, const uint8_t &mkid_adm);
json decAdmKEK(const BIN &mk, const json &adm_json);
json encPKEK(const json &raw_json);
json decPKEK(const json &p_json);
json encDstKEK(const std::string &password, const json &raw_json, unsigned long long opslimit = crypto_pwhash_OPSLIMIT_MODERATE, size_t memlimit = crypto_pwhash_MEMLIMIT_MODERATE);
json decDstKEK(const std::string &password, const json &dst_json);
