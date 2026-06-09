#include "del.h"


// 譁・ｭ怜・縺ｮ螳牙・繧ｼ繝ｭ蛹・
static void secure_zero_string(std::string& s) {
	if (!s.empty()) {
		sodium_memzero(const_cast<char*>(s.data()), s.size());
		s.clear();
		s.shrink_to_fit();
	}
}

// 繝舌う繝翫Μ (vector<uint8_t>) 縺ｮ螳牙・繧ｼ繝ｭ蛹・
static void secure_zero_vector(std::vector<std::uint8_t>& v) {
	if (!v.empty()) {
		sodium_memzero(v.data(), v.size());
		v.clear();
		v.shrink_to_fit();
	}
}

// json 繧貞・蟶ｰ逧・↓襍ｰ譟ｻ縺励※ "豸医☆縺ｹ縺堺ｸｭ霄ｫ" 繧偵ぞ繝ｭ蛹悶☆繧九・
// clear_structure = true 縺ｫ縺吶ｋ縺ｨ譛蠕後↓縺昴・繧ｪ繝悶ず繧ｧ繧ｯ繝郁・菴薙ｒ clear() 縺励※讒矩繧よｶ医☆縲・
void secure_clear_json(json& j, bool clear_structure) {
	// 譁・ｭ怜・
	if (j.is_string()) {
		// get_ref<T&>() 縺ｧ蜀・Κ縺ｮ string 繧貞盾辣ｧ縺ｧ蜿悶ｊ蜃ｺ縺帙ｋ
		try {
			auto& s = j.get_ref<std::string&>();
			secure_zero_string(s);
		} catch (...) {
		}
		return;
	}

	// binary・・son::binary_t == std::vector<uint8_t>・・
	if (j.is_binary()) {
		try {
			auto& b = j.get_ref<json::binary_t&>();
			secure_zero_vector(b);
		} catch (...) {
		}
		return;
	}

	// 驟榊・
	if (j.is_array()) {
		for (auto& el : j) {
			secure_clear_json(el, true);
		}
		if (clear_structure) j.clear();
		return;
	}

	// 繧ｪ繝悶ず繧ｧ繧ｯ繝・
	if (j.is_object()) {
		// 繧､繝・Ξ繝ｼ繧ｿ邨檎罰縺ｧ蛟､縺ｮ蜿ら・繧偵→繧具ｼ医さ繝斐・繧帝∩縺代ｋ・・
		for (auto it = j.begin(); it != j.end(); ++it) {
			secure_clear_json(it.value(), true);
		}
		if (clear_structure) j.clear();
		return;
	}

	// 縺昴・莉・
	if (clear_structure) j.clear();
}
