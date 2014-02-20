/*
 * File description: prim_array_data.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef PRIM_ARRAY_DATA_H_
#define PRIM_ARRAY_DATA_H_

#include <vector>

#include "a_data.h"
#include "prim_data.h"
#include "array_data.h"

namespace axon { namespace serialization {

class AXON_SERIALIZE_API APrimArrayDataBase
	: public AData
{
protected:
	DataType m_innerType;

public:
	typedef std::unique_ptr<APrimArrayDataBase> Ptr;

	APrimArrayDataBase(DataType a_innerType)
		: AData(DataType::PrimArray), m_innerType(a_innerType) { }
	APrimArrayDataBase(DataType a_innerType, CSerializationContext a_context)
		: AData(DataType::PrimArray, std::move(a_context)), m_innerType(a_innerType) { }

	DataType InnerType() const { return m_innerType; }

	virtual size_t Size() const = 0;
	virtual AData::Ptr GetChild(size_t idx) const = 0;
	virtual CArrayData::Ptr ToRegArray() const = 0;

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
};

template<typename T>
class CPrimArrayData
	: public APrimArrayDataBase
{
private:
	typedef std::vector<T> TVec;

	TVec m_children;

public:
	typedef std::unique_ptr<CPrimArrayData<T>> Ptr;

	CPrimArrayData()
		: APrimArrayDataBase(CDataTypeTraits<T>::Type) { }
	CPrimArrayData(CSerializationContext a_context)
		: APrimArrayDataBase(CDataTypeTraits<T>::Type, std::move(a_context)) { }

	template<typename TT>
	void Add(TT &&a_val)
	{
		m_children.push_back(std::forward<TT>(a_val));
	}

	template<typename IterType>
	void Import(IterType a_iter, IterType a_end)
	{
		m_children.insert(m_children.end(), a_iter, a_end);
	}
	template<typename CollType>
	void Import(const CollType &a_coll)
	{
		Import(std::begin(a_coll), std::end(a_coll));
	}

	T &operator[](size_t idx) { return m_children[idx]; }
	const T &operator[](size_t idx) const { return m_children[idx]; }

	T &Get(size_t idx) { return m_children[idx]; }
	const T &Get(size_t idx) const { return m_children[idx]; }

	typename TVec::const_iterator begin() const { return m_children.begin(); }
	typename TVec::iterator begin() { return m_children.begin(); }
	typename TVec::const_iterator end() const { return m_children.end(); }
	typename TVec::iterator end() { return m_children.end(); }
	size_t size() const { return m_children.size(); }

	T *Data() { return m_children.data(); }
	const T *Data() const { return m_children.data(); }

	virtual size_t Size() const override
	{
		return m_children.size();
	}

	virtual AData::Ptr GetChild(size_t idx) const override
	{
		return MakePrim(m_children[idx], Context());
	}

	virtual CArrayData::Ptr ToRegArray() const override
	{
		CArrayData::Ptr l_ret(new CArrayData(Context()));

		for (const T &l_val : m_children)
		{
			l_ret->Add(MakePrim(l_val, Context()));
		}

		return std::move(l_ret);
	}

	virtual std::string ToJsonString(size_t a_numSpaces) const override
	{
		if (m_children.empty())
			return "[]";

		std::ostringstream oss;
		oss << "[" << std::endl;

		std::string l_innerWhite(a_numSpaces + 4, ' ');

		oss << l_innerWhite;
		p_PrintJsonVal(oss, m_children[0]);

		for (auto iter = m_children.cbegin() + 1, end = m_children.end(); iter != end; ++iter)
		{
			oss << "," << std::endl << l_innerWhite;
			p_PrintJsonVal(oss, *iter);
		}

		oss << std::endl << std::string(a_numSpaces, ' ') << "]";
		return oss.str();
	}

private:
	void p_PrintJsonVal(std::ostream &oss, const T &a_val) const
	{
		if (std::is_same<T, std::string>::value)
		{
			oss << "\"";
		}

		oss << a_val;

		if (std::is_same<T, std::string>::value)
		{
			oss << "\"";
		}
	}
};

} };



#endif /* PRIM_ARRAY_DATA_H_ */
