/*
 * File description: null_data.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef NULL_DATA_H_
#define NULL_DATA_H_

#include "a_data.h"

namespace axon { namespace serialization {

#define NULL_CASTER_IMPL(name, type) \
	ADATA_CASTER_FN_IMPL(name, type) { \
		return type(); }

class AXON_SERIALIZE_API CNullData
	: public AData
{
public:
	typedef std::unique_ptr<CNullData> Ptr;

	CNullData() : AData(DataType::Null) { }
	CNullData(CSerializationContext a_context)
		: AData(DataType::Null, std::move(a_context)) { }

	static Ptr Create(CSerializationContext a_context)
	{
		return Ptr(new CNullData(std::move(a_context)));
	}

	NULL_CASTER_IMPL(SByte, sbyte);
	NULL_CASTER_IMPL(UByte, ubyte);
	NULL_CASTER_IMPL(Short, int16_t);
	NULL_CASTER_IMPL(UShort, uint16_t);
	NULL_CASTER_IMPL(Int, int32_t);
	NULL_CASTER_IMPL(UInt, uint32_t);
	NULL_CASTER_IMPL(Long, int64_t);
	NULL_CASTER_IMPL(ULong, uint64_t);
	NULL_CASTER_IMPL(Float, float);
	NULL_CASTER_IMPL(Double, double);
	NULL_CASTER_IMPL(Bool, bool);

	virtual std::string ToString() const override
	{
		return "null";
	}
};

#undef NULL_CASTER_IMPL

} }



#endif /* NULL_DATA_H_ */
