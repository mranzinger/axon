/*
 * File description: data_type.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef DATA_TYPE_H_
#define DATA_TYPE_H_

#include <stdint.h>
#include <string>

#include "util/detail/sfinae_base.h"

namespace axon { namespace serialization {

typedef signed char sbyte;
typedef unsigned char ubyte;

enum class DataType : ubyte
{
	Unknown = 0,
	SByte = 1,
	UByte = 2,
	Short = 3,
	UShort = 4,
	Int = 5,
	UInt = 6,
	Long = 7,
	ULong = 8,
	Float = 9,
	Double = 10,
	String = 11,
	Struct = 12,
	Array = 13,
	Buffer = 14,
	Null = 15,
	Bool = 16,
	PrimArray = 17
};

template<typename T>
struct CDataTypeTraits : util::detail::sfinae_base
{
	// The default data type is an opaque structure
	static const DataType Type = DataType::Struct;
	static const bool IsPrimitive = false;

	typedef no is_primitive_type;
};

#define DTT(type, dataType) \
	template<> \
	struct CDataTypeTraits<type> : util::detail::sfinae_base { \
		static const DataType Type = DataType::dataType; \
		static const bool IsPrimitive = true; \
		typedef yes is_primitive_type; \
	}

DTT(sbyte, SByte);
DTT(ubyte, UByte);
DTT(int16_t, Short);
DTT(uint16_t, UShort);
DTT(int32_t, Int);
DTT(uint32_t, UInt);
DTT(int64_t, Long);
DTT(uint64_t, ULong);
DTT(float, Float);
DTT(double, Double);
DTT(std::string, String);
DTT(bool, Bool);

#undef DTT

} }



#endif /* DATA_TYPE_H_ */
