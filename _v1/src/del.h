#pragma once
#include "def.h"
#include <sodium.h>


template <typename T>
concept HasDataAndSize = requires(T t) {
	{ t.data() } -> std::convertible_to<void*>;
	{ t.size() } -> std::convertible_to<std::size_t>;
} || std::same_as<T, json>;

template <HasDataAndSize Bin>
inline void delm(Bin& bin) {
	sodium_memzero(bin.data(), bin.size());
}

extern void secure_clear_json(json& j, bool clear_structure = true);

inline void delm(json& j) {
	secure_clear_json(j, true);
}

template <HasDataAndSize... Bins>
void delm(Bins&... bins) {
	(delm(bins), ...);
}
