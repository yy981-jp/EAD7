#pragma once

#include <bitset>

namespace awv {
	std::vector<uint8_t> MK_load();
	void MK_clear();
	void MK_unWrap();
	void MK_create();
	void KID_create_write();
	void KID_create_load();
	void KID_recal();
	void KEK_KIDLoad();
	BIN OT_dec(BIN kid);
}
