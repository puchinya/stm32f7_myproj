
#pragma once

#include <kfw_common_base.hpp>
#include <kfw_common_atomic.hpp>

namespace kfw {
	class String;

	using dse_string_t = char8_t *;


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

		bool operator == (const char8_t *z_str) const
		{
			return compare(ConstStringRef(z_str)) == 0;
		}

		bool operator != (const char8_t *z_str) const
		{
			return compare(ConstStringRef(z_str)) != 0;
		}

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

	class StringUtil final : private NonCopyable
	{
	public:
		int32_t compare(const char8_t *s0, size_t s0_len, const char8_t *s1, size_t s1_len);
		bool concat(char8_t *s0, size_t s0_len, size_t s0_max_len,
				const char8_t *s1, size_t s1_len);
	};

	template <size_t L> class StaticString final
	{
	public:
		StaticString() : m_length(0)
		{
		}

		StaticString & operator << (const char8_t *s) {
			size_t s_len = strlen(s);

			concat(m_data, m_length, L, s, s_len);

			return *this;
		}

		bool operator == (const StaticString &s) const {
			return compare(m_data, m_length, s.m_data, s.m_length) == 0;
		}

		bool operator != (const StaticString &s) const {
			return compare(m_data, m_length, s.m_data, s.m_length) != 0;
		}

		char8_t & operator [](size_t index) {
			return m_data[index];
		}

		size_t get_length() const {
			return m_length;
		}

		char8_t *get_data() {
			return &m_data[0];
		}

	private:
		size_t m_length;
		char8_t m_data[L];
	};

	dse_string_t dse_string_create(uint32_t len);
	dse_string_t dse_string_create(const ConstStringRef &s);
	static inline uint32_t dse_string_get_length(const dse_string_t s) {
		return ((uint32_t *)(s - 4))[0];
	}

	void dse_string_addref(dse_string_t s);
	void dse_string_release(dse_string_t s);

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
				m_s = nullptr;
			}
		}

		operator ConstStringRef() const {
			return ConstStringRef(get_data(), get_length());
		}

		const char8_t *get_data() const {
			return m_s;
		}

		String & operator = (const String &obj) {
			if(m_s != nullptr) {
				dse_string_release(m_s);
			}

			m_s = obj.m_s;

			if(m_s != nullptr) {
				dse_string_addref(m_s);
			}

			return *this;
		}

		String & operator = (const char8_t *z_str) {
			if(m_s != nullptr) {
				dse_string_release(m_s);
			}

			m_s = dse_string_create(ConstStringRef(z_str));

			return *this;
		}

		uint32_t get_length() const {
			if(m_s == nullptr) return 0;
			return dse_string_get_length(m_s);
		}

	private:
		dse_string_t m_s;
	};
}
