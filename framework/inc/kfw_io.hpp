/*
 * kfw_io.hpp
 *
 *  Created on: 2019/03/26
 *      Author: masatakanabeshima
 */

#pragma once
#ifndef KFW_IO_HPP
#define KFW_IO_HPP

#include "kfw_common.hpp"

namespace kfw { namespace io {

class Stream : private NonCopyable
{
protected:
	Stream() {}
public:
	virtual ~Stream();

	virtual void flush();
	virtual void close();
	virtual RetVal<uint32_t> read(void *buf, uint32_t size) = 0;
	virtual RetVal<uint32_t> write(const void *buf, uint32_t size) = 0;
};

class BufferedStream final : public Stream
{
public:
	BufferedStream()
	: m_base_stream(nullptr), m_read_buffer(nullptr), m_read_buffer_size(0), m_read_buffer_rp(0), m_read_buffer_wp(0),
	  m_write_buffer(nullptr), m_write_buffer_size(0), m_write_buffer_usage(0) {}

	ret_t init(kfw::io::Stream *base_stream,
			uint8_t *read_buffer, uint32_t read_buffer_size,
			uint8_t *write_buffer, uint32_t write_buffer_size);

	void dispose();

	void flush() override;

	void close() override;

	RetVal<uint32_t> read(void *buf, uint32_t size) override;
	RetVal<uint32_t> write(const void *buf, uint32_t size) override;

private:

	ret_t flush_write_buffer();

	Stream *m_base_stream;
	uint8_t *m_read_buffer;
	uint32_t m_read_buffer_size;
	uint32_t m_read_buffer_rp;
	uint32_t m_read_buffer_wp;
	uint8_t *m_write_buffer;
	uint32_t m_write_buffer_size;
	uint32_t m_write_buffer_usage;
};

}};


#endif /* KFW_IO_HPP_ */
