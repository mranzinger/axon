/*
 * File description: json_serializer.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef JSON_SERIALIZER_H_
#define JSON_SERIALIZER_H_

#include "a_serializer.h"


namespace axon { namespace serialization {

class AXON_SERIALIZE_API CJsonSerializer
	: public ASerializer
{
public:
	virtual std::string FormatName() const override { return "json"; }

	virtual std::string SerializeData(const AData &a_data) const override;

	virtual AData::Ptr DeserializeData(const char *a_buf, const char *a_endBuf) const override;
};

} }



#endif /* JSON_SERIALIZER_H_ */
