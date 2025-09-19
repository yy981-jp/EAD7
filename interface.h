#pragma once
#include <yy981/return.h>

#include "master.h"


inline void createKID(const BIN& mk, uint8_t& mkid mkid, const KIDEntry& kid_e) {
	ordered_json body = loadKID(mk,mkid);
	addNewKid(body,kid_e);
	saveKID(mk,mkid,body);
}
/*
inline KIDEntry getKID(const uint8_t& mkid mkid, const std::string& KIDlabel) {
	for (const ordered_json& e: body.items()) {
		if (e["label"] == KIDlabel) return 
	}
	return_e("");
}
*/