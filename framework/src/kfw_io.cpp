
#include "kfw_io.hpp"

namespace kfw { namespace io {

// --- Stream class ---

Stream::~Stream()
{

}

void Stream::flush()
{
}

void Stream::close()
{
}

// --- BufferedStream class ---

void BufferedStream::flush()
{
	flush_write_buffer();
	m_base_stream->flush();
}

void BufferedStream::close()
{
	flush();
}

ret_t BufferedStream::init(kfw::io::Stream *base_stream,
			uint8_t *read_buffer, uint32_t read_buffer_size,
			uint8_t *write_buffer, uint32_t write_buffer_size)
{
	if(base_stream == nullptr) {
		return kEArgument;
	}

	if(m_base_stream != nullptr) {
		return kEInvalidOperation;
	}

	m_base_stream = base_stream;
	m_read_buffer = read_buffer;
	m_read_buffer_size = read_buffer_size;
	m_write_buffer = write_buffer;
	m_write_buffer_size= write_buffer_size;

	return kOk;
}

void BufferedStream::dispose()
{

}

RetVal<uint32_t> BufferedStream::read(void *buf, uint32_t size)
{
	if(m_base_stream == nullptr) {
		return RetVal<uint32_t>(kEDisposed, 0);
	}

	if(size == 0) {
		return RetVal<uint32_t>(kOk, 0);
	}

	uint8_t *buf_ptr = (uint8_t *)buf;
	uint32_t remain_size = size;
	ret_t ret = kOk;

	while(remain_size > 0) {
		uint32_t remain_in_buffer = m_read_buffer_wp - m_read_buffer_rp;
		if(remain_in_buffer > 0) {
			uint32_t copy_size = size;
			if(copy_size > remain_in_buffer) {
				copy_size = remain_in_buffer;
			}
			memcpy(buf_ptr, &m_read_buffer[m_read_buffer_rp], copy_size);
			buf_ptr += copy_size;
			remain_size -= copy_size;
			m_read_buffer_rp += copy_size;
		} else {

			if(remain_size > m_read_buffer_size) {
				return m_base_stream->read(buf_ptr, remain_size);
			} else {
				auto read_res = m_base_stream->read(m_read_buffer, m_read_buffer_size);
				if(is_failed(read_res.ret)) {
					ret = read_res.ret;
					break;
				}
				m_read_buffer_rp = 0;
				m_read_buffer_wp = read_res.val;
			}
		}
	}

	return RetVal<uint32_t>(ret, size - remain_size);
}

RetVal<uint32_t> BufferedStream::write(const void *buf, uint32_t size)
{
	if(m_base_stream == nullptr) {
		return RetVal<uint32_t>(kEDisposed, 0);
	}

	if(size == 0) {
		return RetVal<uint32_t>(kOk, 0);
	}

	if(m_write_buffer == nullptr) {
		return m_base_stream->write(buf, size);
	}

	ret_t r;

	// バッファより大きいサイズの書き込みの場合はバッファを使用しない。
	if(size > m_write_buffer_size) {
		// 先にバッファをフラッシュする。
		r = flush_write_buffer();
		if(is_failed(r)) {
			return RetVal<uint32_t>(r, 0);
		}
		return m_base_stream->write(buf, size);
	}

	// バッファに空きがなければ、フラッシュする。
	uint32_t free_in_buffer = m_write_buffer_size - m_write_buffer_usage;
	if(free_in_buffer < size) {
		r = flush_write_buffer();
		if(is_failed(r)) {
			return RetVal<uint32_t>(kEDisposed, 0);
		}
	}

	// バッファへ書き込んで終了
	memcpy(&m_write_buffer[m_write_buffer_usage], buf, size);
	m_write_buffer_usage += size;

	return RetVal<uint32_t>(kOk, size);
}

ret_t BufferedStream::flush_write_buffer()
{
	if(m_write_buffer_usage == 0) {
		return kOk;
	}

	auto write_res = m_base_stream->write(m_write_buffer, m_write_buffer_usage);
	if(is_failed(write_res.ret)) {
		return write_res.ret;
	}

	m_write_buffer_usage = 0;

	return kOk;
}

// --- StreamWriter class ---

StreamWriter & StreamWriter::operator << (const ConstStringRef &value) {
	ret_t r;
	r = m_s.write(value.get_data(), value.get_length()).ret;
	if(is_failed(r)) {
		set_error(r);
	}
	return *this;
}

StreamWriter & StreamWriter::operator << (const char8_t *value) {
	ret_t r;
	uint32_t slen = strlen(value);
	r = m_s.write(value, slen).ret;
	if(is_failed(r)) {
		set_error(r);
	}
	return *this;
}

StreamWriter & StreamWriter::operator << (int32_t value) {
	char8_t s[Int32::kMaxStringLength];
	uint32_t slen;
	ret_t r = Int32::to_string(value, s, Int32::kMaxStringLength, slen);
	if(is_failed(r)) {
		set_error(r);
	} else {
		r = m_s.write(s, slen).ret;
		if(is_failed(r)) {
			set_error(r);
		}
	}

	return *this;
}

StreamWriter & StreamWriter::operator << (uint32_t value) {
	char8_t s[UInt32::kMaxStringLength];
	uint32_t slen;
	ret_t r = UInt32::to_string(value, s, UInt32::kMaxStringLength, slen);
	if(is_failed(r)) {
		set_error(r);
	} else {
		r = m_s.write(s, slen).ret;
		if(is_failed(r)) {
			set_error(r);
		}
	}

	return *this;
}

}}
