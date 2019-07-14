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
	virtual ~Stream() {}

	virtual void flush() {}
	virtual void close() {}
	virtual RetVal<uint32_t> read(void *buf, size_t size) = 0;
	virtual RetVal<uint32_t> write(const void *buf, size_t size) = 0;
};


}};


#endif /* KFW_IO_HPP_ */
