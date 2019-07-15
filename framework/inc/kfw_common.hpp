
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
		return 0x8000000 | (module_id << 16) | err_code;
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
	constexpr ret_t kEUnknown = make_err_ret(kComponentCommon, 255);

	struct NonCopyable {
		NonCopyable() = default;
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator=(const NonCopyable&) = delete;
	};

	void kfw_assert(const char *msg);
	void kfw_die();

	class ConstStringRef final
	{
	public:
		explicit ConstStringRef(const char8_t *c_str)
		{
			m_data = c_str;
			m_length = strlen(c_str);
		}

		ConstStringRef(const char8_t *data, uint32_t length) :
			m_data(data), m_length(length)
		{
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
};

#endif
