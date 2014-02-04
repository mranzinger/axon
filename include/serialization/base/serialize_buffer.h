/*
 * File description: serialize_buffer.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef SERIALIZE_BUFFER_H_
#define SERIALIZE_BUFFER_H_

#include "util/buffer.h"

namespace axon { namespace serialization {

template<>
struct CSerializer<util::CBuffer>
{
public:
	static AData::Ptr Get(const util::CBuffer &a_val, const CSerializationContext &a_context)
	{
		return AData::Ptr(new CBufferData(a_val, a_context.IsCompressSet(), a_context));
	}
};

} }


#endif /* SERIALIZE_BUFFER_H_ */
