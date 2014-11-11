/*
 * File description: axon_serializer.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef AXON_SERIALIZER_H_
#define AXON_SERIALIZER_H_

#include "a_serializer.h"

namespace axon { namespace serialization {

class AXON_SERIALIZE_API CAxonSerializer
	: public ASerializer
{
public:
	virtual std::string FormatName() const override { return "axon"; }

	virtual size_t CalcSize(const AData &a_data) const override;

	virtual size_t SerializeInto(const AData &a_data, char *a_buffer, size_t a_bufferSize) const override;

	virtual std::string SerializeData(const AData &a_data) const override;

	virtual AData::Ptr DeserializeData(const char *a_buf, const char *a_endBuf) const override;

private:
	size_t p_EstablishSize(const AData &a_data) const;
};

} }



#endif /* AXON_SERIALIZER_H_ */
