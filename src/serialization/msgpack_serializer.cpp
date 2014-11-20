/*
 * msgpack_serializer.cpp
 *
 *  Created on: Nov 19, 2014
 *      Author: mranzinger
 */

#include "serialization/format/msgpack_serializer.h"

#include <unordered_map>
#include <type_traits>

using namespace std;

typedef unsigned char byte;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulong;

namespace axon { namespace serialization {

//--------- Format Definitions ----------------------
const byte POS_FIX_INT = 0;
const byte FIX_MAP = 0x80;
const byte FIX_ARRAY = 0x90;
const byte FIX_STR = 0xa0;
const byte NIL = 0xc0;
const byte FALSE = 0xc2;
const byte TRUE = 0xc3;
const byte BIN_8 = 0xc4;
const byte BIN_16 = 0xc5;
const byte BIN_32 = 0xc6;
const byte FLOAT_32 = 0xca;
const byte FLOAT_64 = 0xcb;
const byte UINT_8 = 0xcc;
const byte UINT_16 = 0xcd;
const byte UINT_32 = 0xce;
const byte UINT_64 = 0xcf;
const byte INT_8 = 0xd0;
const byte INT_16 = 0xd1;
const byte INT_32 = 0xd2;
const byte INT_64 = 0xd3;
const byte STR_8 = 0xd9;
const byte STR_16 = 0xda;
const byte STR_32 = 0xdb;
const byte ARR_16 = 0xdc;
const byte ARR_32 = 0xdd;
const byte MAP_16 = 0xde;
const byte MAP_32 = 0xdf;
const byte NEG_FIX_INT = 0xe0;
//--------------------------------------------------

template<typename IntType, bool IsSigned>
struct int_helper_t;



template<typename NumType, bool IsIntegral>
struct num_helper_t;

// Integer Types
template<typename NumType>
struct num_helper_t<NumType, true>
    : int_helper_t<NumType, is_signed<NumType>::value>
{};

template<typename T>
void WriteValueImpl(char *&a_buff, const typename enable_if<is_trivial<T>::value, T>::type &a_val)
{
    memcpy(a_buff, &a_val, sizeof(T));
    a_buff += sizeof(T);
}

template<typename T>
void WriteValue(char *&a_buff, const T &a_val)
{
    WriteValueImpl<T>(a_buff, a_val);
}

template<byte Code, typename FloatType>
struct float_helper_tt;

template<typename FloatType>
struct float_helper_t;

template<>
struct float_helper_t<float>
    : float_helper_tt<FLOAT_32, float>
{};

template<>
struct float_helper_t<double>
    : float_helper_tt<FLOAT_64, double>
{};

// Floating Point Types
template<typename NumType>
struct num_helper_t<NumType, false>
    : float_helper_t<NumType>
{};

// Signed Integers
template<typename IntType>
struct int_helper_t<IntType, true>
{
    static size_t CalcSize(IntType val)
    {
        if (val < 0)
        {
            IntType v2 = -val;

            if (v2 <= IntType(0x1f))
                return sizeof(byte);

            // Have to catch this case because the positive number
            // case can pack more bits
            if (v2 <= IntType(0xff))
                return 2 * sizeof(byte);

            // Just use the same size semantics as the signed version.
            // Must use v2 because it is positive
            return int_helper_t<IntType, false>::CalcSize(v2);
        }
        else
        {
            // This number is positive, so just treat it as unsigned
            return int_helper_t<IntType, false>::CalcSize(val);
        }
    }

    static void Encode(char *&a_buff, IntType val)
    {
        if (val < 0)
        {
            IntType v2 = -val;

            if (v2 <= IntType(0x1f))
            {
                WriteValue(a_buff, NEG_FIX_INT | byte(v2));
            }
            else if (v2 <= IntType(0x7f))
            {
                WriteValue(a_buff, INT_8);
                WriteValue(a_buff, int8_t(val));
            }
            else if (v2 <= IntType(0x7fff))
            {
                WriteValue(a_buff, INT_16);
                WriteValue(a_buff, int16_t(val));
            }
            else if (v2 <= IntType(0x7fffffff))
            {
                WriteValue(a_buff, INT_32);
                WriteValue(a_buff, int32_t(val));
            }
            else
            {
                WriteValue(a_buff, INT_64);
                WriteValue(a_buff, int64_t(val));
            }
        }
        else
        {
            int_helper_t<IntType, false>::Encode(a_buff, val);
        }
    }
};

// Unsigned Integers
template<typename IntType>
struct int_helper_t<IntType, false>
{
    static size_t CalcSize(IntType val)
    {
        // positive fixint
        if (val <= IntType(0x7f))
            return sizeof(byte);

        // uint_8
        if (val <= IntType(0xff))
            return 2 * sizeof(byte);

        // uint_16
        if (val <= IntType(0xffff))
            return 3 * sizeof(byte);

        // uint_32
        if (val <= IntType(0xffffffff))
            return 5 * sizeof(byte);

        return 9 * sizeof(byte);
    }

    static void Encode(char *&a_buff, IntType val)
    {
        if (val <= IntType(0x7f))
        {
            WriteValue(a_buff, POS_FIX_INT | byte(val));
        }
        else if (val <= IntType(0xff))
        {
            WriteValue(a_buff, UINT_8);
            WriteValue(a_buff, byte(val));
        }
        else if (val <= IntType(0xffff))
        {
            WriteValue(a_buff, UINT_16);
            WriteValue(a_buff, uint16_t(val));
        }
        else if (val <= IntType(0xffffffff))
        {
            WriteValue(a_buff, UINT_32);
            WriteValue(a_buff, uint32_t(val));
        }
        else
        {
            WriteValue(a_buff, UINT_64);
            WriteValue(a_buff, uint64_t(val));
        }
    }

};

// Floating Point
template<byte Code, typename FloatType>
struct float_helper_tt
{
    static size_t CalcSize(FloatType)
    {
        return sizeof(byte) + sizeof(FloatType);
    }

    static void Encode(char *&a_buff, FloatType val)
    {
        WriteValue(a_buff, Code);
        WriteValue(a_buff, val);
    }
};

template<typename NumType>
struct num_helper
    : num_helper_t<NumType, is_integral<NumType>::value>
{};

template<typename T>
struct prim_helper
    : num_helper<T> { };

template<>
struct prim_helper<bool>
{
    static size_t CalcSize(bool) { return 1; }

    static void Encode(char *&a_buff, bool v)
    {
        WriteValue(a_buff, v ? TRUE : FALSE);
    }
};

size_t CalcRawSize(size_t len)
{
    if (len <= 31)
    {
        return len + 1;
    }
    else if (len <= 0xff)
    {
        return len + 2;
    }
    else if (len <= 0xffff)
    {
        return len + 3;
    }
    else if (len <= 0xffffffff)
    {
        return len + 5;
    }
    else
    {
        throw runtime_error("Cannot represent strings with lengths larger than (2^32)-1");
    }
}

void WriteRawBytes(char *&a_opBuff, const char *a_ipBuff, size_t a_len)
{
    memcpy(a_opBuff, a_ipBuff, a_len);
    a_opBuff += a_len;
}

template<>
struct prim_helper<string>
{
    static size_t CalcSize(const string &a_val)
    {
        return CalcRawSize(a_val.size());
    }

    static void Encode(char *&a_buff, const string &a_val)
    {
        if (a_val.size() <= 0x1f)
        {
            WriteValue(a_buff, FIX_STR | byte(a_val.size()));
        }
        else if (a_val.size() <= 0xff)
        {
            WriteValue(a_buff, STR_8);
            WriteValue(a_buff, byte(a_val.size()));
        }
        else if (a_val.size() <= 0xffff)
        {
            WriteValue(a_buff, STR_16);
            WriteValue(a_buff, uint16_t(a_val.size()));
        }
        else
        {
            WriteValue(a_buff, STR_32);
            WriteValue(a_buff, uint32_t(a_val.size()));
        }

        WriteRawBytes(a_buff, a_val.c_str(), a_val.size());
    }
};

// The default assumption here is that T is a primitive type
template<typename T>
size_t CalcSize(const T &a_val)
{
    return prim_helper<T>::CalcSize(a_val);
}

size_t p_CalcStructSize(const CStructData &a_data);
size_t p_CalcArraySize(const CArrayData &a_data);
size_t p_CalcBufferSize(const CBufferData &a_data);
size_t p_CalcPrimArraySize(const APrimArrayDataBase &a_data);

size_t p_CalcSize(const AData &a_data)
{
#define PRIM_SIZE(name, type) \
    case DataType::name: \
        return CalcSize(static_cast<const CPrimData<type> &>(a_data).GetValue())

    switch (a_data.Type())
    {
    PRIM_SIZE(SByte, byte);
    PRIM_SIZE(UByte, byte);
    PRIM_SIZE(Short, short);
    PRIM_SIZE(UShort, ushort);
    PRIM_SIZE(Int, int);
    PRIM_SIZE(UInt, uint);
    PRIM_SIZE(Long, long long);
    PRIM_SIZE(ULong, ulong);
    PRIM_SIZE(Float, float);
    PRIM_SIZE(Double, double);
    PRIM_SIZE(Bool, bool);
    PRIM_SIZE(String, string);

    case DataType::Null:
        return 1;

    case DataType::Struct:
        return p_CalcStructSize(static_cast<const CStructData &>(a_data));

    case DataType::Array:
        return p_CalcArraySize(static_cast<const CArrayData &>(a_data));

    case DataType::Buffer:
        return p_CalcBufferSize(static_cast<const CBufferData &>(a_data));

    case DataType::PrimArray:
        return p_CalcPrimArraySize(static_cast<const APrimArrayDataBase &>(a_data));

    default:
        throw runtime_error("Unknown serialization type.");
    }

#undef PRIM_SIZE
}

size_t p_CalcStructSize(const CStructData &a_data)
{
    size_t len = 0;

    if (a_data.size() <= 0xf)
    {
        len = 1;
    }
    else if (a_data.size() <= 0xffff)
    {
        len = 3;
    }
    else if (a_data.size() <= 0xffffffff)
    {
        len = 5;
    }
    else
    {
        throw runtime_error("Cannot encode more map elements than (2^32)-1");
    }

    for (const CStructData::TProp &l_prop : a_data)
    {
        len += CalcSize(l_prop.first);
        len += p_CalcSize(*l_prop.second);
    }

    return len;
}

size_t p_CalcArraySize(const CArrayData &a_data)
{
    size_t len = 0;

    if (a_data.size() <= 0xf)
    {
        len = 1;
    }
    else if (a_data.size() <= 0xffff)
    {
        len = 3;
    }
    else if (a_data.size() <= 0xffffffff)
    {
        len = 5;
    }
    else
    {
        throw runtime_error("Cannot encode more array elements than (2^32)-1");
    }

    for (const AData::Ptr &l_data : a_data)
    {
        len += p_CalcSize(*l_data);
    }

    return len;
}

template<typename T>
size_t p_CalcPrimArraySizeImpl(const CPrimArrayData<T> &a_data)
{
    size_t len = 0;

    if (a_data.size() <= 0xf)
    {
        len = 1;
    }
    else if (a_data.size() <= 0xffff)
    {
        len = 3;
    }
    else if (a_data.size() <= 0xffffffff)
    {
        len = 5;
    }
    else
    {
        throw runtime_error("Cannot encode more array elements than (2^32)-1");
    }

    for (const T &l_val : a_data)
    {
        len += CalcSize(*l_val);
    }

    return len;
}

size_t p_CalcPrimArraySize(const APrimArrayDataBase &a_data)
{
#define CALC_SIZE(name, type) \
    case DataType::name: \
        return p_CalcPrimArraySizeImpl(static_cast<const CPrimArrayData<type> &>(a_data))

    switch (a_data.InnerType())
    {
    CALC_SIZE(SByte, signed char);
    CALC_SIZE(UByte, unsigned char);
    CALC_SIZE(Short, short);
    CALC_SIZE(UShort, unsigned short);
    CALC_SIZE(Int, int);
    CALC_SIZE(UInt, unsigned int);
    CALC_SIZE(Long, long long);
    CALC_SIZE(ULong, unsigned long);
    CALC_SIZE(Float, float);
    CALC_SIZE(Double, double);
    CALC_SIZE(Bool, bool);
    CALC_SIZE(String, string);

    default:
        throw runtime_error("Unsupported primitive type.");
    }

#undef CALC_SIZE
}

size_t p_CalcBufferSize(const CBufferData &a_data)
{
    return CalcRawSize(a_data.BufferSize());
}

size_t CMsgPackSerializer::CalcSize(const AData &a_data) const
{
    return p_CalcSize(a_data);
}



} }
