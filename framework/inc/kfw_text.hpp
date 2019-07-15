#pragma once
#ifndef KFW_TEXT_HPP
#define KFW_TEXT_HPP

#include <kfw_common.hpp>
#include <kfw_system.hpp>

namespace kfw { namespace text {

class StringBuffer final : private NonCopyable
{
public:
	StringBuffer(uint32_t capacity);
	~StringBuffer();

	char8_t at(uint32_t index) const {
		if(index >= m_length) {
			kfw_system_error(kEIndexOutOfRange);
		}
		return m_data[index];
	}

	ret_t append(int ch);

	uint32_t get_length() const {
		return m_length;
	}
private:
	uint32_t	m_capacity;
	uint32_t	m_length;
	char8_t		*m_data;
};

}}

#endif
