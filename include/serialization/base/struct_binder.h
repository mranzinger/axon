/*
 * File description: struct_binder.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef STRUCT_BINDER_H_
#define STRUCT_BINDER_H_

#include "a_data.h"
#include "prim_data.h"
#include "array_data.h"
#include "struct_data.h"

namespace axon { namespace serialization {

namespace detail {

struct unspecialized { };

}

template<typename Binder, typename T>
detail::unspecialized BindStruct(Binder &a_data, T &a_val);

struct CStructWriter;
struct CStructReader;

template<typename T>
auto WriteStruct(CStructWriter &a_writer, const T &a_val) -> decltype(BindStruct(a_writer, const_cast<T&>(a_val)))
{
	return BindStruct(a_writer, const_cast<T&>(a_val));
}

template<typename T>
auto ReadStruct(const CStructReader &a_reader, T &a_val) -> decltype(BindStruct(a_reader, a_val))
{
	return BindStruct(a_reader, a_val);
}


template<typename T>
AData::Ptr Serialize(const T &, const CSerializationContext &);

struct CStructWriter
{
	CStructData *m_data;

	CStructWriter(CStructData *a_data)
		: m_data(a_data) { }

	template<typename T, SerializationFlags ...Flags>
	CStructWriter &operator()(std::string a_name, const T &a_val)
	{
		const auto &l_flags = SetSerFlags<Flags...>(m_data->Context());

		m_data->Add(std::move(a_name),
				    Serialize(a_val, m_data->Context()));

		return *this;
	}
};

struct CStructReader
{
	const CStructData *m_data;

	CStructReader(const CStructData *a_data)
		: m_data(a_data) { }

	template<typename T, SerializationFlags ...Flags>
	const CStructReader &operator()(const std::string &a_name, T &a_val) const
	{
		const auto &l_flags = SetSerFlags<Flags...>(m_data->Context());

		Deserialize(m_data->Get(a_name), a_val);

		return *this;
	}
};

} }


#endif /* STRUCT_BINDER_H_ */
