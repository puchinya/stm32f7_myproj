
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

}
