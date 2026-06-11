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
