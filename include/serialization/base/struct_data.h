/*
 * File description: struct_data.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef STRUCT_DATA_H_
#define STRUCT_DATA_H_

#include <vector>
#include <map>
#include <algorithm>

#include "a_data.h"


namespace axon { namespace serialization {

class AXON_SERIALIZE_API CStructData
	: public AData
{
private:
	typedef std::pair<std::string, AData::Ptr> TProp;
	typedef std::vector<TProp> TVec;

	TVec m_props;

public:
	typedef std::unique_ptr<CStructData> Ptr;

	CStructData();
	CStructData(CSerializationContext a_context);

	void Add(std::string a_name, AData::Ptr a_val);

	void Set(const std::string &a_name, AData::Ptr a_val);

	const AData::Ptr &Get(const std::string &a_name) const;

	AData *Find(const std::string &a_name) const;

	TVec::const_iterator begin() const { return m_props.begin(); }
	TVec::iterator begin() { return m_props.begin(); }
	TVec::const_iterator end() const { return m_props.end(); }
	TVec::iterator end() { return m_props.end(); }
	size_t size() const { return m_props.size(); }


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



#endif /* STRUCT_DATA_H_ */
