#ifndef _UTIL_CRC32C_TABLES_H_
#define _UTIL_CRC32C_TABLES_H_

// Copyright 2008,2009,2010 Massachusetts Institute of Technology.
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#include <stdint.h>

namespace axon { namespace util { namespace detail {

extern const uint32_t crc_tableil8_o32[256];
extern const uint32_t crc_tableil8_o40[256];
extern const uint32_t crc_tableil8_o48[256];
extern const uint32_t crc_tableil8_o56[256];
extern const uint32_t crc_tableil8_o64[256];
extern const uint32_t crc_tableil8_o72[256];
extern const uint32_t crc_tableil8_o80[256];
extern const uint32_t crc_tableil8_o88[256];

} } }

#endif
