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

class CStructBinder;
class CStructWriter;
class CStructReader;

template<typename T>
detail::unspecialized BindStruct(const CStructBinder &a_binder, T &a_val);

template<typename T>
auto WriteStruct(const CStructWriter &a_writer, const T &a_val) -> decltype(BindStruct(&a_writer, const_cast<T&>(a_val)))
{
	return BindStruct(&a_writer, const_cast<T&>(a_val));
}

template<typename T>
auto ReadStruct(const CStructReader &a_reader, T &a_val) -> decltype(BindStruct(&a_reader, a_val))
{
	return BindStruct(&a_reader, a_val);
}


template<typename T>
AData::Ptr Serialize(const T &, const CSerializationContext &);
template<typename T>
T Deserialize(const AData &a_data);

class CStructWriter
{
private:
	CStructData *m_data;

public:
	CStructWriter(CStructData *a_data)
		: m_data(a_data) { }

	template<typename T, typename ...Flags>
	const CStructWriter &operator()(std::string a_name, const T &a_val, Flags ...a_flags) const
	{
		const auto &l_flags = SetSerFlags(m_data->Context(), a_flags...);

		m_data->Add(std::move(a_name),
				    Serialize(a_val, m_data->Context()));

		return *this;
	}
};

class CStructReader
{
private:
	const CStructData *m_data;

public:
	CStructReader(const CStructData *a_data)
		: m_data(a_data) { }

	template<typename T>
	T GetPrimitive(const std::string &a_name) const
	{
		return Deserialize<T>(*m_data->Get(a_name));
	}

	template<typename T, typename ...Flags>
	const CStructReader &operator()(const std::string &a_name, T &a_val, Flags ...a_flags) const
	{
		const auto &l_flags = SetSerFlags(m_data->Context(), a_flags...);

		Deserialize(m_data->Get(a_name), a_val);

		return *this;
	}
};

class CStructBinder
{
private:
	const CStructWriter *m_writer = nullptr;
	const CStructReader *m_reader = nullptr;

public:
	CStructBinder(const CStructWriter *a_writer) : m_writer(a_writer) { }
	CStructBinder(const CStructReader *a_reader) : m_reader(a_reader) { }

	template<typename T, typename ...Flags>
	const CStructBinder &operator()(const std::string &a_name, T &a_val, Flags ...a_flags) const
	{
		if (m_writer)
			(*m_writer)(a_name, a_val, a_flags...);
		else
			(*m_reader)(a_name, a_val, a_flags...);

		return *this;
	}
};

} }


#endif /* STRUCT_BINDER_H_ */
