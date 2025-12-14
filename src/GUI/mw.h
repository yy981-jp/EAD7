#pragma once
#include <string>

namespace mw {
	extern INP_FROM inp_from;
	void setInpFrom(const INP_FROM& inp_from_new);
	void import_dst_kek(const QString& qstr, bool from_kek_window = false);
	void textProc(const std::string& text);
	void fileProc(const std::string& path);
	void run();
}
