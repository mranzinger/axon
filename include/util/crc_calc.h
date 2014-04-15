/*
 * File description: crc_calc.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef CRC_CALC_H_
#define CRC_CALC_H_

#include <stdint.h>
#include <stdlib.h>
#include <numeric>

namespace axon { namespace util {

uint32_t CalcCRC32(const void *a_data, size_t a_size);

} }



#endif /* CRC_CALC_H_ */
