#pragma once
#include <type_traits>

namespace string_encryption {
	template<class _Ty>
	using clean_type = typename std::remove_const_t<std::remove_reference_t<_Ty>>;

	template <int _size, char _key1, char _key2, typename T>
	class crypter {
	public:
		__forceinline constexpr crypter(T* data) {
			crypt(data);
		}

		__forceinline T* get() {
			return _storage;
		}

		__forceinline int size() {
			return _size;
		}

		__forceinline  char key() {
			return _key1;
		}

		__forceinline  T* encrypt() {
			if (!is_encrypted())
				crypt(_storage);

			return _storage;
		}

		__forceinline  T* decrypt() {
			if (is_encrypted())
				crypt(_storage);

			return _storage;
		}

		__forceinline bool is_encrypted() {
			return _storage[_size - 1] != 0;
		}

		__forceinline void clear() {
			for (int i = 0; i < _size; i++) {
				_storage[i] = 0;
			}
		}

		__forceinline operator T* () {
			decrypt();

			return _storage;
		}

	private:
		__forceinline constexpr void crypt(T* data) {
			for (int i = 0; i < _size; i++) {
				_storage[i] = data[i] ^ (_key1 + i % (1 + _key2));
			}
		}

		T _storage[_size]{};
	};
}

#define str(str) str_key(str, __TIME__[4], __TIME__[7]).decrypt( )
#define str_key(str, key1, key2) []() { \
			constexpr static auto crypted = string_encryption::crypter \
				<sizeof(str) / sizeof(str[0]), key1, key2, string_encryption::clean_type<decltype(str[0])>>((string_encryption::clean_type<decltype(str[0])>*)str); \
					return crypted; }()