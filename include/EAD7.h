#pragma once
#incldue "../src/base.h"

namespace EAD7 {
	extern BIN enc(const BIN& kek, const BIN& plaintext, const uint8_t& mkid, const BIN& kid);
	extern BIN dec(const BIN& kek, const BIN& blob);
}
