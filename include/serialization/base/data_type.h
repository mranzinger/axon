/*
 * File description: data_type.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef DATA_TYPE_H_
#define DATA_TYPE_H_

#include <stdint.h>
#include <string>

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
	Bool = 16
};

template<typename T>
struct CDataTypeTraits
{
	// The default data type is an opaque structure
	static const DataType Type = DataType::Struct;
	static const bool IsPrimitive = false;
};

#define DTT(type, dataType) \
	template<> \
	struct CDataTypeTraits<type> { \
		static const DataType Type = DataType::dataType; \
		static const bool IsPrimitive = true; \
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
