/*
 * File description: prim_data.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef PRIM_DATA_H_
#define PRIM_DATA_H_

#include <string>
#include <string.h>
#include <sstream>

#include "a_data.h"

namespace axon { namespace serialization {



template<typename A, typename B>
struct CPrimCaster
{
	static B Get(const A &a_val) { return static_cast<B>(a_val); }
};

template<typename B>
struct CPrimCaster<std::string, B>
{
	static B Get(const std::string &a_val)
	{
		B l_ret;

		std::istringstream ss(a_val);
		ss >> l_ret;

		if (ss.bad())
			throw std::runtime_error("Invalid Conversion.");

		return l_ret;
	}
};

template<typename B>
struct CPrimCaster<const char *, B>
	: CPrimCaster<std::string, B>
{

};

template<typename A>
struct CPrimCaster<A, std::string>
{
	static std::string Get(const A &a_val)
	{
		std::ostringstream oss;
		oss << a_val;

		if (oss.bad())
			throw std::runtime_error("Invalid Conversion.");

		return oss.str();
	}
};

template<>
struct AXON_SERIALIZE_API CPrimCaster<std::string, std::string>
{
	static const std::string &Get(const std::string &a_val)
	{
		return a_val;
	}
};

template<>
struct AXON_SERIALIZE_API CPrimCaster<const char *, std::string>
	: CPrimCaster<std::string, std::string>
{
};

template<typename B, typename A>
auto cast_to(const A &a_val) -> decltype(CPrimCaster<A, B>::Get(a_val))
{
	return CPrimCaster<A, B>::Get(a_val);
}

#define PRIM_CASTER_IMPL(name, type) \
	ADATA_CASTER_FN_IMPL(name, type) { \
		return cast_to<type>(m_value); }

template<typename T>
class CPrimData
	: public AData
{
private:
	T m_value;

public:
	typedef std::unique_ptr<CPrimData<T>> Ptr;

	CPrimData()
		: AData(CDataTypeTraits<T>::Type) { }
	CPrimData(T a_value)
		: AData(CDataTypeTraits<T>::Type),
		  m_value(std::move(a_value))
	{
	}
	CPrimData(T a_value, CSerializationContext a_context)
		: AData(CDataTypeTraits<T>::Type, std::move(a_context)),
		  m_value(std::move(a_value))
    {

    }

	T &GetValue() { return m_value; }
	const T &GetValue() const { return m_value; }

	void SetValue(T a_val)
	{
		m_value = std::move(a_val);
	}


	PRIM_CASTER_IMPL(SByte, sbyte);
	PRIM_CASTER_IMPL(UByte, ubyte);
	PRIM_CASTER_IMPL(Short, int16_t);
	PRIM_CASTER_IMPL(UShort, uint16_t);
	PRIM_CASTER_IMPL(Int, int32_t);
	PRIM_CASTER_IMPL(UInt, uint32_t);
	PRIM_CASTER_IMPL(Long, int64_t);
	PRIM_CASTER_IMPL(ULong, uint64_t);
	PRIM_CASTER_IMPL(Float, float);
	PRIM_CASTER_IMPL(Double, double);
	PRIM_CASTER_IMPL(Bool, bool);
	PRIM_CASTER_IMPL(String, std::string);
};

template<typename T>
typename CPrimData<T>::Ptr MakePrim(T a_value, CSerializationContext a_context = CSerializationContext())
{
	return typename CPrimData<T>::Ptr(new CPrimData<T>(std::move(a_value), std::move(a_context)));
}

inline CPrimData<std::string>::Ptr MakePrim(const char *a_value, CSerializationContext a_context = CSerializationContext())
{
	return CPrimData<std::string>::Ptr(new CPrimData<std::string>(a_value, std::move(a_context)));
}

typedef CPrimData<std::string> CStringData;
typedef CPrimData<bool> CBoolData;

} }

#undef PRIM_CASTER_IMPL

#endif /* PRIM_DATA_H_ */
