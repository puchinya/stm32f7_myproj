
#include "kfw_common.hpp"
#include "kfw_system.hpp"

namespace kfw {

void kfw_throw(ret_t r)
{
	kfw_system_error(r);
}

int32_t ConstStringRef::compare(const ConstStringRef &obj) const
{
	int32_t cmp = (int32_t)(m_length - obj.m_length);
	if(cmp != 0) { return cmp; }
	if(m_length == 0) {
		return 0;
	}
	return (int32_t)memcmp(m_data, obj.m_data, m_length);
}

int32_t ConstStringRef::compare_ignore_case(const ConstStringRef &obj) const
{
	int32_t cmp = (int32_t)(m_length - obj.m_length);
	if(cmp != 0) { return cmp; }
	if(m_length == 0) {
		return 0;
	}
	for(uint32_t i = 0; i < m_length; i++) {
		int a = m_data[i];
		int b = obj.m_data[i];

		if('A' <= a && a <= 'Z') {
			a = a - 'A' + 'a';
		}
		if('A' <= b && b <= 'Z') {
			b = b - 'A' + 'a';
		}

		cmp = a - b;
		if(cmp != 0) {
			return cmp;
		}
	}

	return 0;
}

int32_t ConstStringRef::index_of(int32_t ch, uint32_t start_index) const
{
	uint32_t p;
	for(p = start_index; p < m_length; p++) {
		if(m_data[p] == ch) {
			return (int32_t)p;
		}
	}
	return -1;
}

int32_t ConstStringRef::index_of(const ConstStringRef &obj, uint32_t start_index) const
{
	if(obj.m_length == 0) {
		return 0;
	}

	if(m_length == 0) {
		return -1;
	}

	if(m_length < obj.m_length) {
		return -1;
	}

	for(uint32_t i = start_index; i < m_length - (obj.m_length - 1); i++) {
		if(m_data[i] == obj.m_data[0]) {
			for(uint32_t j = 0; j < obj.m_length; j++) {
				const char8_t *data = &m_data[i];
				if(data[j] != obj.m_data[j]) {
					goto next;
				}
			}
			return (int32_t)i;
		}
next:
		continue;
	}
	return -1;
}

ConstStringRef ConstStringRef::trim() const
{

	if(m_length == 0) {
		return *this;
	}

	uint32_t h, e;
	for(h = 0; h < m_length; h++) {
		if(m_data[h] != ' ') {
			break;
		}
	}

	if(h == m_length) {
		return ConstStringRef();
	}

	for(e = m_length; e > 0; e--) {
		if(m_data[e - 1] != ' ') {
			break;
		}
	}

	return ConstStringRef(&m_data[h], e - h);
}

//--- Int32 class ---
ret_t Int32::parse(const ConstStringRef &s, int32_t &val)
{
	val = 0;
	if(s.get_length() == 0) {
		return kEFormat;
	}

	int32_t sign = s[0] == '-' ? -1 : 1;

	int32_t r = 0;
	for(uint32_t p = 0; p != s.get_length(); p++)
	{
		int32_t ch = s[p];
		if(ch < '0' || ch > '9') {
			return kEFormat;
		}
		int n = ch - '0';
		r = r *10 + n;
	}
	val = r * sign;
	return kOk;
}

ret_t Int32::to_string(int32_t value, char8_t *str_buf, uint32_t str_buf_len, uint32_t &str_len)
{
	char8_t buf[11];
	int32_t sign = value;
	if(value < 0) value = -value;
	uint32_t len = 0;
	do {
		uint32_t digit = value % 10;
		buf[len++] = digit + '0';
		value /= 10;
	} while(value != 0);

	if(len > str_buf_len) {
		str_len = 0;
		return kEArgument;
	}

	if(sign < 0) {
		buf[len++] = '-';
	}

	for(uint32_t i = 0; i < len; i++) {
		str_buf[i] = buf[len - i - 1];
	}

	str_len = len;

	return kOk;
}

//--- UInt32 class ---
ret_t UInt32::parse(const ConstStringRef &s, uint32_t &val)
{
	val = 0;
	if(s.get_length() == 0) {
		return kEFormat;
	}

	uint32_t r = 0;
	for(uint32_t p = 0; p != s.get_length(); p++)
	{
		int32_t ch = s[p];
		if(ch < '0' || ch > '9') {
			return kEFormat;
		}
		int n = ch - '0';
		r = r *10 + n;
	}

	val = r;

	return kOk;
}

ret_t UInt32::to_string(uint32_t value, char8_t *str_buf, uint32_t str_buf_len, uint32_t &str_len)
{
	char8_t buf[10];
	uint32_t len = 0;
	do {
		uint32_t digit = value % 10;
		buf[len++] = digit + '0';
		value /= 10;
	} while(value != 0);

	if(len > str_buf_len) {
		str_len = 0;
		return kEArgument;
	}

	for(uint32_t i = 0; i < len; i++) {
		str_buf[i] = buf[len - i - 1];
	}

	str_len = len;

	return kOk;
}

}
