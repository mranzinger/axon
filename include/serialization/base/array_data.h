/*
 * File description: coll_data.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef COLL_DATA_H_
#define COLL_DATA_H_

#include <vector>

#include "a_data.h"

namespace axon { namespace serialization {

class AXON_SERIALIZE_API CArrayData
	: public AData
{
private:
	typedef std::vector<AData::Ptr> TVec;

	TVec m_children;

public:
	typedef std::unique_ptr<CArrayData> Ptr;

	CArrayData();
	CArrayData(CSerializationContext a_context);

	void Add(AData::Ptr a_val);

    void Erase(size_t idx);

    const AData::Ptr& operator [](size_t idx) const;

    const AData::Ptr& Get(size_t idx) const;

	TVec::const_iterator begin() const { return m_children.begin(); }
	TVec::iterator begin() { return m_children.begin(); }
	TVec::const_iterator end() const { return m_children.end(); }
	TVec::iterator end() { return m_children.end(); }
	size_t size() const { return m_children.size(); }

	ADATA_CASTER_FN_NOT_IMPL(SByte, sbyte);
	ADATA_CASTER_FN_NOT_IMPL(UByte, ubyte);
	ADATA_CASTER_FN_NOT_IMPL(Short, int16_t);
	ADATA_CASTER_FN_NOT_IMPL(UShort, uint16_t);
	ADATA_CASTER_FN_NOT_IMPL(Int, int32_t);
	ADATA_CASTER_FN_NOT_IMPL(UInt, uint32_t);
	ADATA_CASTER_FN_NOT_IMPL(Long, int64_t);
	ADATA_CASTER_FN_NOT_IMPL(ULong, uint64_t);
	ADATA_CASTER_FN_NOT_IMPL(Float, float);
	ADATA_CASTER_FN_NOT_IMPL(Double, double);
	ADATA_CASTER_FN_NOT_IMPL(String, std::string);
	ADATA_CASTER_FN_NOT_IMPL(Bool, bool);

    virtual std::string ToJsonString(size_t a_numSpaces) const override;
};



} }



#endif /* COLL_DATA_H_ */
