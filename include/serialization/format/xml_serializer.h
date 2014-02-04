/*
 * File description: xml_serializer.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef XML_SERIALIZER_H_
#define XML_SERIALIZER_H_

#include "a_serializer.h"

namespace axon { namespace serialization {

class CXmlSerializer
	: public ASerializer
{
protected:
	virtual std::string SerializeData(const AData &a_data) const override;

	virtual AData::Ptr DeserializeData(const char *a_buf, const char *a_endBuf) const override;
};

} }



#endif /* XML_SERIALIZER_H_ */
