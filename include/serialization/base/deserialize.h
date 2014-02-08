/*
 * File description: deserialize.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef DESERIALIZE_H_
#define DESERIALIZE_H_

#include <type_traits>

#include "a_data.h"
#include "prim_data.h"
#include "array_data.h"
#include "struct_data.h"
#include "null_data.h"
#include "buffer_data.h"

#include "util/or.h"
#include "util/can_stream_in.h"

#include "struct_binder.h"

namespace axon { namespace serialization {

template<typename T, bool CanStreamIn>
struct CStreamDeserializer
{
	static_assert(sizeof(T) == 0,
			"\n\n\n\n"
			"***********************************************************************************************************************\n"
			"The specified type must do one of the following to be deserializable: \n"
			"(1) Be a primitive data type, \n"
			"(2) Create a specialized ReadStruct function with signature 'void ReadStruct(const CStructReader &, [your type] &)', \n"
			"(3) Create a specialized BindStruct function with signature 'template<Binder> void BindStruct(Binder &, [your type] &)',\n"
			"(4) Implement the stream input operator to std::istream\n"
			"***********************************************************************************************************************\n"
			"\n\n\n\n");

	static void Deserialize(const AData &a_data, T &a_val);
};

template<typename T, bool Specialized>
struct CSpecDeserializer
{
	static void Deserialize(const AData &a_data, T &a_val)
	{
		CStreamDeserializer<T, util::can_stream_in<T>::Value>::Deserialize(a_data, a_val);
	}
};

template<typename T, bool IsPrimitive>
struct CPrimitiveDeserializer
{
	static void Deserialize(const AData &a_data, T &a_val)
	{
		CSpecDeserializer<T,
			util::exp_or<
				!std::is_same<detail::unspecialized, decltype(ReadStruct(*(CStructReader*)nullptr, *(T*)nullptr))>::value,
				!std::is_same<detail::unspecialized, decltype(BindStruct(*(CStructBinder*)nullptr, *(T*)nullptr))>::value
				>::Value>::Deserialize(a_data, a_val);
	}
};

template<typename T>
struct CStreamDeserializer<T, true>
{
	static void Deserialize(const AData &a_data, T &a_val)
	{
		a_val = cast_to<T>(a_data.ToString());
	}
};

template<typename T>
struct CSpecDeserializer<T, true>
{
	static void Deserialize(const AData &a_data, T &a_val)
	{
		if (a_data.Type() != DataType::Struct)
			throw std::runtime_error("There is a mismatch between the serialization and deserialization mechanisms."
					" Ensure that the same method is used both ways.");

		auto l_struct = static_cast<const CStructData *>(&a_data);

		CStructReader l_reader(l_struct);

		ReadStruct(l_reader, a_val);
	}
};

template<typename T>
struct CPrimitiveDeserializer<T, true>
{
	static void Deserialize(const AData &a_data, T &a_val)
	{
		a_val = (T)a_data;
	}
};

template<typename T>
struct CDeserializer
{
	static void Deserialize(const AData &a_data, T &a_val)
	{
		CPrimitiveDeserializer<T, CDataTypeTraits<T>::IsPrimitive>::Deserialize(a_data, a_val);
	}
};

template<typename T>
void Deserialize(const AData &a_data, T &a_val)
{
	CDeserializer<T>::Deserialize(a_data, a_val);
}

template<typename T>
T Deserialize(const AData &a_data)
{
	T l_ret;
	Deserialize(a_data, l_ret);
	return l_ret;
}

template<typename T>
void Deserialize(const AData::Ptr &a_data, T &a_val)
{
	Deserialize(*a_data, a_val);
}

template<typename T>
T Deserialize(const AData::Ptr &a_data)
{
	return Deserialize<T>(*a_data);
}

} }

#include "deserialize_ptr.h"
#include "deserialize_coll.h"
#include "deserialize_buffer.h"


#endif /* DESERIALIZE_H_ */
