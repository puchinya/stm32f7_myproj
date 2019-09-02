
#pragma once

#include <kfw_common_base.hpp>
#include <kfw_common_string.hpp>

namespace kfw {

	class Int32 final
	{
	public:
		static constexpr uint32_t kMaxStringLength = 11;

		Int32(int32_t value) : m_value(value) {}
		Int32() : m_value(0) {}

		static ret_t parse(const ConstStringRef &s, int32_t &val);
		static ret_t to_string(int32_t value, char8_t *str_buf, uint32_t str_buf_len, uint32_t &str_len);

	private:
		int32_t	m_value;
	};

	class UInt32 final
	{
	public:
		static constexpr uint32_t kMaxStringLength = 10;

		UInt32(uint32_t value) : m_value(value) {}
		UInt32() : m_value(0) {}

		static ret_t parse(const ConstStringRef &s, uint32_t &val);
		static ret_t to_string(uint32_t value, char8_t *str_buf, uint32_t str_buf_len, uint32_t &str_len);

	private:
		uint32_t	m_value;
	};
}
