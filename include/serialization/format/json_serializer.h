/*
 * File description: json_serializer.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef JSON_SERIALIZER_H_
#define JSON_SERIALIZER_H_

#include "a_serializer.h"


namespace axon { namespace serialization {

class CJsonSerializer
	: public ASerializer
{
public:
	virtual std::string FormatName() const override { return "json"; }

protected:
	virtual std::string SerializeData(const AData &a_data) const override;

	virtual AData::Ptr DeserializeData(const char *a_buf, const char *a_endBuf) const override;
};

} }



#endif /* JSON_SERIALIZER_H_ */
