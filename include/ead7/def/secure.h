#include <sodium.h>
#include <concepts>

#include <ead7/def/def.h>
#include <ead7/def/json.h>
#include <ead7/def/bin.h>


EAD7_BEGIN


template <typename T>
concept HasDataAndSize = requires(T t) {
	{ t.data() } -> std::convertible_to<void*>;
	{ t.size() } -> std::convertible_to<std::size_t>;
} || std::same_as<T, json>;

template <HasDataAndSize Bin>
inline void delm(Bin& bin) {
	sodium_memzero(bin.data(), bin.size());
}


EAD7_END
