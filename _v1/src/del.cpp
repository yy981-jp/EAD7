#include "del.h"


// 文字列の安全ゼロ化
static void secure_zero_string(std::string& s) {
	if (!s.empty()) {
		sodium_memzero(const_cast<char*>(s.data()), s.size());
		s.clear();
		s.shrink_to_fit();
	}
}

// バイナリ (vector<uint8_t>) の安全ゼロ化
static void secure_zero_vector(std::vector<std::uint8_t>& v) {
	if (!v.empty()) {
		sodium_memzero(v.data(), v.size());
		v.clear();
		v.shrink_to_fit();
	}
}

// json を再帰的に走査して "消すべき中身" をゼロ化する。
// clear_structure = true にすると最後にそのオブジェクト自体を clear() して構造も消す。
void secure_clear_json(json& j, bool clear_structure) {
	// 文字列
	if (j.is_string()) {
		// get_ref<T&>() で内部の string を参照で取り出せる
		try {
			auto& s = j.get_ref<std::string&>();
			secure_zero_string(s);
		} catch (...) {
		}
		return;
	}

	// binary（json::binary_t == std::vector<uint8_t>）
	if (j.is_binary()) {
		try {
			auto& b = j.get_ref<json::binary_t&>();
			secure_zero_vector(b);
		} catch (...) {
		}
		return;
	}

	// 配列
	if (j.is_array()) {
		for (auto& el : j) {
			secure_clear_json(el, true);
		}
		if (clear_structure) j.clear();
		return;
	}

	// オブジェクト
	if (j.is_object()) {
		// イテレータ経由で値の参照をとる（コピーを避ける）
		for (auto it = j.begin(); it != j.end(); ++it) {
			secure_clear_json(it.value(), true);
		}
		if (clear_structure) j.clear();
		return;
	}

	// その他
	if (clear_structure) j.clear();
}
