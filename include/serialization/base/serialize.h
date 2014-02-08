/*
 * File description: serialize.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef SERIALIZE_H_
#define SERIALIZE_H_

#include <type_traits>

#include "a_data.h"
#include "prim_data.h"
#include "array_data.h"
#include "struct_data.h"
#include "null_data.h"
#include "buffer_data.h"
#include "prim_array_data.h"

#include "util/or.h"
#include "util/can_stream_out.h"

#include "struct_binder.h"

namespace axon { namespace serialization {



template<typename T, bool CanStreamOut>
struct CStreamSerializer
{
	static_assert(sizeof(T) == 0,
			"\n\n\n\n"
			"***********************************************************************************************************************\n"
			"The specified type must do one of the following to be serializable: \n"
			"(1) Be a primitive data type, \n"
			"(2) Create a specialized WriteStruct function with signature 'void WriteStruct(CStructWriter &, const [your type] &)', \n"
			"(3) Create a specialized BindStruct function with signature 'template<Binder> void BindStruct(Binder &, [your type] &)',\n"
			"(4) Implement the stream output operator to std::ostream\n"
			"***********************************************************************************************************************\n"
			"\n\n\n\n");

	static AData::Ptr Get(const T &, const CSerializationContext &) { return AData::Ptr(); }
};

template<typename T>
struct CStreamSerializer<T, true>
{
	static AData::Ptr Get(const T &a_val, const CSerializationContext &a_context)
	{
		return AData::Ptr(new CPrimData<std::string>(cast_to<std::string>(a_val), a_context));
	}
};

template<typename T, bool Specialized>
struct CSpecSerializer
{
	// The standard definition is implicitly false
	static AData::Ptr Get(const T &a_val, const CSerializationContext &a_context)
	{
		// Ok, so the type isn't primitive, and it also doesn't have a specialized
		// structure writing routine, so try to stream it out
		return CStreamSerializer<T, util::can_stream_out<T>::Value>::Get(a_val, a_context);
	}
};

template<typename T>
struct CSpecSerializer<T, true>
{
	static AData::Ptr Get(const T &a_val, const CSerializationContext &a_context)
	{
		CStructData::Ptr l_ret(new CStructData(a_context));
		CStructWriter l_writer(l_ret.get());

		WriteStruct(l_writer, a_val);

		return std::move(l_ret);
	}
};

template<typename T, bool IsPrimitive>
struct CPrimitiveSerializer
{
	// The standard definition is implicitly false
	// because true will be specialized

	static AData::Ptr Get(const T &a_val, const CSerializationContext &a_context)
	{
		return CSpecSerializer<T,
				util::exp_or<
					!std::is_same<detail::unspecialized, decltype(WriteStruct(*(CStructWriter*)nullptr, *(T*)nullptr))>::value,
					!std::is_same<detail::unspecialized, decltype(BindStruct(*(CStructBinder*)nullptr, *(T*)nullptr))>::value
					>::Value>::Get(a_val, a_context);
	}
};

template<typename T>
struct CPrimitiveSerializer<T, true>
{
	static AData::Ptr Get(const T &a_val, const CSerializationContext &a_context)
	{
		return AData::Ptr(new CPrimData<T>(a_val, a_context));
	}
};

template<typename T>
struct CSerializer
{
	static AData::Ptr Get(const T &a_val, const CSerializationContext &a_context)
	{
		return CPrimitiveSerializer<T, CDataTypeTraits<T>::IsPrimitive>::Get(a_val, a_context);
	}
};

template<typename T>
AData::Ptr Serialize(const T &a_val, const CSerializationContext &a_context)
{
	return CSerializer<T>::Get(a_val, a_context);
}

template<typename T>
AData::Ptr Serialize(const T &a_val)
{
	return Serialize(a_val, CSerializationContext());
}

} }

#include "serialize_ptr.h"
#include "serialize_coll.h"
#include "serialize_buffer.h"


#endif /* SERIALIZE_H_ */
