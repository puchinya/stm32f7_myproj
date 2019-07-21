
#include "kfw_text.hpp"

namespace kfw { namespace text {

StringBuffer::StringBuffer(uint32_t capacity)
{
	char8_t *mem = new char8_t[capacity];

	m_capacity = capacity;
	m_length = 0;
	m_data = mem;
}

StringBuffer::~StringBuffer()
{
	if(m_data != nullptr) {
		delete [] m_data;
		m_data = nullptr;
	}
}

ret_t StringBuffer::append(int ch)
{
	if(m_length >- m_capacity)
		return kEOutOfMemory;
	m_data[m_length++] = (char8_t)ch;

	return kOk;
}

}}

