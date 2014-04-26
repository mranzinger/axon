/*
 * File description: crc_calc.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include "util/crc_calc.h"

#include "detail/crc32c.h"

namespace axon { namespace util {

uint32_t CalcCRC32(const void *a_data, size_t a_size)
{
	uint32_t l_ret = detail::crc32c(detail::crc32cInit(), a_data, a_size);

	return l_ret;
}

} }

/*#include <boost/crc.hpp>

namespace axon { namespace util {

typedef boost::crc_optimal<64, 0x1EDC6F41, 0, 0, true, true> crc_type;

uint64_t CalcCRC64(const void* a_data, size_t a_size)
{
	crc_type crc;

	crc.process_bytes(a_data, a_size);

	return crc.checksum();
}

} }*/
