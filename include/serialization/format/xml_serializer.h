/*
 * File description: xml_serializer.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef XML_SERIALIZER_H_
#define XML_SERIALIZER_H_

#include "a_serializer.h"

namespace axon { namespace serialization {

class AXON_SERIALIZE_API CXmlSerializer
	: public ASerializer
{
public:
	virtual std::string FormatName() const override { return "xml"; }

	virtual std::string SerializeData(const AData &a_data) const override;

	virtual AData::Ptr DeserializeData(const char *a_buf, const char *a_endBuf) const override;
};

} }



#endif /* XML_SERIALIZER_H_ */
