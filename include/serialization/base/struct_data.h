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

	CStructData()
		: AData(DataType::Struct) { }
	CStructData(CSerializationContext a_context)
		: AData(DataType::Struct, std::move(a_context)) { }

	void Add(std::string a_name, AData::Ptr a_val)
	{
		m_props.emplace_back(std::move(a_name), std::move(a_val));
	}

	void Set(const std::string &a_name, AData::Ptr a_val)
	{
		auto iter = std::find_if(m_props.begin(), m_props.end(),
				[&a_name] (const TProp &a_prop)
				{
					return a_name == a_prop.first;
				});

		if (iter == m_props.end())
			Add(a_name, std::move(a_val));
		else
			iter->second = std::move(a_val);
	}

	const AData::Ptr &Get(const std::string &a_name) const
	{
		static const AData::Ptr s_missing;

		auto iter = std::find_if(m_props.begin(), m_props.end(),
				[&a_name] (const TProp &a_prop)
				{
					return a_name == a_prop.first;
				});

		if (iter != m_props.end())
			return iter->second;
		else
			return s_missing;
	}

	AData *Find(const std::string &a_name) const
	{
		auto iter = std::find_if(m_props.begin(), m_props.end(),
				[&a_name] (const TProp &a_prop)
				{
					return a_name == a_prop.first;
				});

		if (iter != m_props.end())
			return iter->second.get();
		else
			return nullptr;
	}

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

	virtual std::string ToJsonString(size_t a_numSpaces) const override
	{
		std::ostringstream oss;
		oss << "{" << std::endl;

		std::string l_spaces(a_numSpaces + 4, ' ');

		for (auto iter = m_props.begin(), end = m_props.end(); iter != end; ++iter)
		{
			const TProp &l_prop = *iter;

			oss << l_spaces << "\"" <<
				   l_prop.first << "\" : " <<
				   l_prop.second->ToJsonString(a_numSpaces + l_prop.first.size() + 4);

			if (iter != (end - 1))
				oss << ", ";
			oss << std::endl;
		}
		oss << std::string(a_numSpaces, ' ') << "}";
		return oss.str();
	}
};

} }



#endif /* STRUCT_DATA_H_ */
