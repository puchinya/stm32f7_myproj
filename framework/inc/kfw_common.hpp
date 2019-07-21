
#pragma once
#ifndef KFW_COMMON_HPP
#define KFW_COMMON_HPP

#include <stddef.h>
#include <stdint.h>
#include <string.h>

using char8_t = char;

#define KFW_ASSERT(expr, ret) if(!(expr)) { kfw::kfw_assert(#expr); }
#define interface struct

namespace kfw {
	using ret_t = int;
	using component_id_t = int;

	template<class T> struct RetVal {
		RetVal(ret_t ret_, T val_)
		: ret(ret_), val(val_) {
		}
		ret_t ret;
		T val;
	};

	constexpr component_id_t kComponentCommon = 0;

	constexpr ret_t make_err_ret(component_id_t module_id, int err_code)
	{
		return (ret_t)((uint32_t)0x80000000 | ((uint32_t)module_id << 16) | (uint32_t)err_code);
	}

	static inline bool is_success(ret_t r) {
		return r >= 0;
	}

	static inline bool is_failed(ret_t r) {
		return r < 0;
	}

	constexpr ret_t kOk = 0;
	constexpr ret_t kEArgument = make_err_ret(kComponentCommon, 1);
	constexpr ret_t kEIndexOutOfRange = make_err_ret(kComponentCommon, 2);
	constexpr ret_t kEOutOfMemory = make_err_ret(kComponentCommon, 3);
	constexpr ret_t kEFormat = make_err_ret(kComponentCommon, 4);
	constexpr ret_t kETimeout = make_err_ret(kComponentCommon, 5);
	constexpr ret_t kEInvalidOperation = make_err_ret(kComponentCommon, 6);
	constexpr ret_t kEResource = make_err_ret(kComponentCommon, 7);
	constexpr ret_t kEDisposed = make_err_ret(kComponentCommon, 8);
	constexpr ret_t kENotSupported = make_err_ret(kComponentCommon, 9);
	constexpr ret_t kEUnknown = make_err_ret(kComponentCommon, 255);

	struct NonCopyable {
		NonCopyable() = default;
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator=(const NonCopyable&) = delete;
	};

	void kfw_assert(const char *msg);
	[[noreturn]] void kfw_throw(ret_t r);

	template <typename T, size_t L> class StaticArray;
	template <typename T> class ArrayRef;
	template <typename T> class ConstArrayRef;

	template <typename T, size_t L> class StaticArray final : private NonCopyable
	{
		friend class ArrayRef<T>;
		friend class ConstArrayRef<T>;
	public:
		StaticArray() = default;

		T &operator [](size_t index) {
			return m_items[index];
		}

		size_t get_length() const {
			return L;
		}

		size_t get_byte_size() const {
			return L * sizeof(T);
		}

		operator T *() {
			return m_items;
		}

	private:
		T m_items[L];
	};

	template <typename T> class ArrayRef final
	{
		friend class ConstArrayRef<T>;
	public:
		ArrayRef(T *data, size_t length)
		: m_data(data), m_length(length) {}
		ArrayRef(const ArrayRef &obj)
		: m_data(obj.m_data), m_length(obj.m_length) {}
		template <size_t L> ArrayRef(const StaticArray<T, L> &obj)
		: m_data(obj.m_items), m_length(L) {}

		T &operator [](size_t index) {
			return m_data[index];
		}

		operator const T *() const {
			return m_data;
		}

		operator T *() {
			return m_data;
		}

		size_t get_length() const {
			return m_length;
		}

		size_t get_byte_size() const {
			return m_length * sizeof(T);
		}

	private:
		T *m_data;
		size_t m_length;
	};

	template <typename T> class ConstArrayRef final
	{
	public:
		ConstArrayRef(const T *data, size_t length)
		: m_data(data), m_length(length) {}
		ConstArrayRef(const ConstArrayRef<T> &obj)
		: m_data(obj.m_data), m_length(obj.m_length) {}
		ConstArrayRef(const ArrayRef<T> &obj)
		: m_data(obj.m_data), m_length(obj.m_length) {}
		template <size_t L> ConstArrayRef(const StaticArray<T, L> &obj)
		: m_data(obj.m_items), m_length(L) {}

		const T &operator [](size_t index) const {
			return m_data[index];
		}

		operator const T *() const {
			return m_data;
		}

		size_t get_length() const {
			return m_length;
		}

		size_t get_byte_size() const {
			return m_length * sizeof(T);
		}

	private:
		const T *m_data;
		size_t m_length;
	};

	class ConstStringRef final
	{
	public:
		ConstStringRef() : m_data(nullptr), m_length(0) {}
		ConstStringRef(const ConstStringRef &obj) : m_data(obj.m_data), m_length(obj.m_length) {}
		explicit ConstStringRef(const char8_t *c_str)
		{
			m_data = c_str;
			m_length = strlen(c_str);
		}

		ConstStringRef(const char8_t *data, uint32_t length) :
			m_data(data), m_length(length)
		{
		}

		void assign(const char8_t *data, uint32_t length)
		{
			m_data = data;
			m_length = length;
		}

		ConstStringRef &operator = (const ConstStringRef &obj)
		{
			m_data = obj.m_data;
			m_length = obj.m_length;

			return *this;
		}

		int32_t operator [] (uint32_t index) const {
			return m_data[index];
		}

		int32_t at(uint32_t index) const {
			return m_data[index];
		}

		int32_t compare(const char8_t *c_str) const
		{
			return compare(ConstStringRef(c_str));
		}

		int32_t compare(const ConstStringRef &obj) const;

		int32_t compare_ignore_case(const char8_t *c_str) const
		{
			return compare_ignore_case(ConstStringRef(c_str));
		}

		int32_t compare_ignore_case(const ConstStringRef &obj) const;

		int32_t index_of(int32_t ch, uint32_t start_index = 0) const;

		int32_t index_of(const ConstStringRef &obj, uint32_t start_index = 0) const;

		ConstStringRef sub_string(uint32_t index, uint32_t length) const {
			return ConstStringRef(&m_data[index], length);
		}

		ConstStringRef sub_string(uint32_t index) const {
			auto length = m_length - index;
			return ConstStringRef(&m_data[index], length);
		}

		ConstStringRef trim() const;

		bool operator == (const ConstStringRef &obj) const
		{
			return compare(obj) == 0;
		}

		bool operator != (const ConstStringRef &obj) const
		{
			return compare(obj) != 0;
		}

		const char8_t *get_data() const {
			return m_data;
		}
		const uint32_t get_length() const {
			return m_length;
		}
	private:
		const char8_t	*m_data;
		uint32_t		m_length;
	};

	class String;

	using dse_string_t = char8_t *;

	static inline dse_string_t dse_string_create(uint32_t len) {
		uint8_t *p = new uint8_t[len + 8];
		if(p == nullptr) {
			return nullptr;
		}
		return (dse_string_t)(p + 8);
	}
	static inline dse_string_t dse_string_create(const ConstStringRef &s) {
		uint8_t *p = new uint8_t[s.get_length() + 8];
		if(p == nullptr) {
			return nullptr;
		}
		uint8_t *str_top = p + 8;
		memcpy(str_top, s.get_data(), s.get_length());
		return (dse_string_t)str_top;
	}
	static inline uint32_t dse_string_get_length(const dse_string_t s) {
		return ((uint32_t *)(s - 4))[0];
	}
	static inline void dse_string_addref(dse_string_t s) {
		((uint32_t *)(s - 8))[0]++;
	}
	static inline void dse_string_release(dse_string_t s) {
		uint32_t *p = (uint32_t *)(s - 8);
		uint32_t cnt = --p[0];
		if(cnt == 0) {
			delete p;
		}
	}

	class String final
	{
	public:

		String() : m_s(nullptr) {}

		String(const ConstStringRef &s) {
			m_s = dse_string_create(s);
			if(m_s == nullptr) {
				kfw_throw(kEOutOfMemory);
			}
		}

		String(const String &obj) {
			dse_string_t ss = obj.m_s;
			if(ss != nullptr) {
				dse_string_addref(ss);
				m_s = ss;
			} else {
				m_s = nullptr;
			}
		}

		~String() {
			if(m_s != nullptr) {
				dse_string_release(m_s);
			}
		}

		operator ConstStringRef() const {
			return ConstStringRef(get_data(), get_length());
		}

		const char8_t *get_data() const {
			return m_s;
		}

		uint32_t get_length() const {
			if(m_s == nullptr) return 0;
			return dse_string_get_length(m_s);
		}

	private:
		dse_string_t m_s;
	};


};

#endif
