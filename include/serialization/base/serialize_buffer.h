/*
 * File description: serialize_buffer.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef SERIALIZE_BUFFER_H_
#define SERIALIZE_BUFFER_H_

#include "util/buffer.h"

namespace axon { namespace serialization {

template<>
struct AXON_SERIALIZE_API CSerializer<util::CBuffer>
{
public:
	static AData::Ptr Get(const util::CBuffer &a_val, const CSerializationContext &a_context)
	{
		return AData::Ptr(new CBufferData(a_val, a_context.IsCompressSet(), a_context));
	}
};

} }


#endif /* SERIALIZE_BUFFER_H_ */
