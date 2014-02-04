/*
 * File description: deserialize_buffer.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef DESERIALIZE_BUFFER_H_
#define DESERIALIZE_BUFFER_H_

#include "util/buffer.h"

#include "buffer_data.h"

namespace axon { namespace serialization {

template<>
struct CDeserializer<util::CBuffer>
{
	static void Deserialize(const AData &a_data, util::CBuffer &a_buff)
	{
		if (DataType::Buffer == a_data.Type())
		{
			a_buff = static_cast<const CBufferData *>(&a_data)->GetBuffer();
		}
		else
		{
			std::string l_enc = a_data.ToString();

			a_buff.Reset(l_enc.size());

			std::copy(l_enc.begin(), l_enc.end(), a_buff.begin());
		}
	}
};

} }



#endif /* DESERIALIZE_BUFFER_H_ */
