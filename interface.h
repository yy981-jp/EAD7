#pragma once
#include <yy981/return.h>


inline void createKID(const BIN& mk, int mkid, const KIDEntry& kid_e) {
	ordered_json body = loadKID(mk,mkid);
	addNewKid(body,kid_e);
	saveKID(mk,mkid,body);
}

inline KIDEntry getKID(const int mkid, const std::string& KIDlabel) {
	for (const ordered_json& e: body.items()) {
		if (e["label"] == KIDlabel) return 
	}
	return_e();
}