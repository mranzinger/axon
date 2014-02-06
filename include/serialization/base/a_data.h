/*
 * File description: a_base.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef A_BASE_H_
#define A_BASE_H_

#include <memory>
#include <stdexcept>

#include "data_type.h"
#include "serialization_context.h"
#include "i_data_context.h"

namespace axon { namespace serialization {

#define ADATA_CASTER_FN(name, type) virtual type To ## name() const
#define ADATA_CASTER_FN_ABS(name, type) ADATA_CASTER_FN(name, type) = 0
#define ADATA_CASTER_FN_IMPL(name, type) ADATA_CASTER_FN(name, type) override
#define ADATA_CASTER_FN_NOT_IMPL(name, type) \
	ADATA_CASTER_FN_IMPL(name, type) { \
		throw std::runtime_error("Not Supported."); }

template<typename T>
struct ADataCaster;

class AData
{
private:
	const DataType m_type;
	CSerializationContext m_context;
	mutable IDataContext::Ptr m_dataContext;

public:
	typedef std::unique_ptr<AData> Ptr;

	virtual ~AData()
	{
	}

	DataType Type() const { return m_type; }

	template<typename T>
	explicit operator T() const
	{
		return ADataCaster<T>::Get(this);
	}

	const CSerializationContext &Context() const { return m_context; }
	CSerializationContext &Context() { return m_context; }

	ADATA_CASTER_FN_ABS(SByte, sbyte);
	ADATA_CASTER_FN_ABS(UByte, ubyte);
	ADATA_CASTER_FN_ABS(Short, int16_t);
	ADATA_CASTER_FN_ABS(UShort, uint16_t);
	ADATA_CASTER_FN_ABS(Int, int32_t);
	ADATA_CASTER_FN_ABS(UInt, uint32_t);
	ADATA_CASTER_FN_ABS(Long, int64_t);
	ADATA_CASTER_FN_ABS(ULong, uint64_t);
	ADATA_CASTER_FN_ABS(Float, float);
	ADATA_CASTER_FN_ABS(Double, double);
	ADATA_CASTER_FN_ABS(String, std::string);
	ADATA_CASTER_FN_ABS(Bool, bool);

	virtual std::string ToJsonString(size_t a_spaceIndent) const
	{
		// Initialize with spaces prefixed
		std::string l_ret;

		if (m_type == DataType::String)
			l_ret += "\"";

		l_ret += ToString();

		if (m_type == DataType::String)
			l_ret += "\"";

		return l_ret;
	}
	std::string ToJsonString() const { return ToJsonString(0); }

	IDataContext *GetDataContext() const
	{
		return m_dataContext.get();
	}
	void SetDataContext(IDataContext::Ptr a_context) const
	{
		m_dataContext = std::move(a_context);
	}

protected:
	AData(DataType a_type) : m_type(a_type) { }
	AData(DataType a_type, CSerializationContext a_context)
		: m_type(a_type), m_context(std::move(a_context)) { }
};



#define ADATA_CASTER_S(name, type) \
	template<> \
	struct ADataCaster<type> { \
		static type Get(const AData *a_data) { return a_data->To ## name(); } \
	}

ADATA_CASTER_S(SByte, sbyte);
ADATA_CASTER_S(UByte, ubyte);
ADATA_CASTER_S(Short, int16_t);
ADATA_CASTER_S(UShort, uint16_t);
ADATA_CASTER_S(Int, int32_t);
ADATA_CASTER_S(UInt, uint32_t);
ADATA_CASTER_S(Long, int64_t);
ADATA_CASTER_S(ULong, uint64_t);
ADATA_CASTER_S(Float, float);
ADATA_CASTER_S(Double, double);
ADATA_CASTER_S(String, std::string);
ADATA_CASTER_S(Bool, bool);

#undef ADATA_CASTER_S
#undef ADATA_CASTER_FN_ABS
// Leaving the impl definition for subclasses

} }


#endif /* A_BASE_H_ */
