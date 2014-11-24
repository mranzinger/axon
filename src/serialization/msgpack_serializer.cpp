/*
 * msgpack_serializer.cpp
 *
 *  Created on: Nov 19, 2014
 *      Author: mranzinger
 */

#include "serialization/format/msgpack_serializer.h"

#include <unordered_map>
#include <type_traits>
#include <assert.h>
#include <endian.h>

using namespace std;

typedef unsigned char byte;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulong;

namespace axon { namespace serialization {

namespace {

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

const bool IS_BIG_ENDIAN = BYTE_ORDER == BIG_ENDIAN;

template<typename T, bool IsBigEndian>
struct value_writer_t;

template<typename T>
struct value_writer_t<T, true>
{
    static_assert(is_trivial<T>::value, "The value to be written must be trivial.");

    static void WriteValue(char *&a_buff, const T &a_val)
    {
        memcpy(a_buff, &a_val, sizeof(T));
        a_buff += sizeof(T);
    }
    static void ReadValue(const char *&a_buff, T &a_val)
    {
        memcpy(&a_val, a_buff, sizeof(a_val));
        a_buff += sizeof(a_val);
    }
};

template<typename T>
struct value_writer_t<T, false>
{
    static_assert(is_trivial<T>::value, "The value to be written must be trivial.");

    template<size_t Size>
    struct re
    {
        static void WriteValue(char *&a_buff, const T &a_val)
        {
            auto l_buff = (const char *)&a_val;

            Go(l_buff, a_buff);
            a_buff += sizeof(T);
        }
        static void ReadValue(const char *&a_buff, T &a_val)
        {
            auto l_buff = (char *)&a_val;

            Go(a_buff, l_buff);
            a_buff += sizeof(T);
        }

        static void Go(const char *a_src, char *a_dest)
        {
            #pragma unroll
            for (size_t i = 0; i < Size; ++i)
            {
                a_dest[i] = a_src[Size - i - 1];
            }
        }
    };

    static void WriteValue(char *&a_buff, const T &a_val)
    {
        re<sizeof(T)>::WriteValue(a_buff, a_val);
    }
    static void ReadValue(const char *&a_buff, T &a_val)
    {
        re<sizeof(T)>::ReadValue(a_buff, a_val);
    }
};

template<typename T>
struct value_writer
    : value_writer_t<T, IS_BIG_ENDIAN> { };

template<typename T>
void WriteValue(char *&a_buff, const T &a_val)
{
    value_writer<T>::WriteValue(a_buff, a_val);
}

template<typename T>
void ReadValue(const char *&a_buff, T &a_val)
{
    value_writer<T>::ReadValue(a_buff, a_val);
}

template<typename T>
T ReadValue(const char *&a_buff)
{
    T l_val;
    ReadValue(a_buff, l_val);
    return l_val;
}

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
                WriteValue<uint8_t>(a_buff, NEG_FIX_INT | (int8_t(val) & 0x1f));
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
            WriteValue<uint8_t>(a_buff, POS_FIX_INT | byte(val));
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

inline size_t CalcRawSize(size_t len)
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

inline void WriteRawBytes(char *&a_opBuff, const char *a_ipBuff, size_t a_len)
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
            WriteValue<uint8_t>(a_buff, FIX_STR | byte(a_val.size()));
        }
        else if (a_val.size() <= 0xff)
        {
            WriteValue(a_buff, STR_8);
            WriteValue<uint8_t>(a_buff, a_val.size());
        }
        else if (a_val.size() <= 0xffff)
        {
            WriteValue(a_buff, STR_16);
            WriteValue<uint16_t>(a_buff, a_val.size());
        }
        else
        {
            WriteValue(a_buff, STR_32);
            WriteValue<uint32_t>(a_buff, a_val.size());
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

template<typename T>
void WritePrimitive(char *&a_buff, const T &a_val)
{
    prim_helper<T>::Encode(a_buff, a_val);
}

size_t CalcStructSize(const CStructData &a_data);
size_t CalcArraySize(const CArrayData &a_data);
size_t CalcBufferSize(const CBufferData &a_data);
size_t CalcPrimArraySize(const APrimArrayDataBase &a_data);

inline size_t CalcDataSize(const AData &a_data)
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
        return CalcStructSize(static_cast<const CStructData &>(a_data));

    case DataType::Array:
        return CalcArraySize(static_cast<const CArrayData &>(a_data));

    case DataType::Buffer:
        return CalcBufferSize(static_cast<const CBufferData &>(a_data));

    case DataType::PrimArray:
        return CalcPrimArraySize(static_cast<const APrimArrayDataBase &>(a_data));

    default:
        throw runtime_error("Unknown serialization type.");
    }

#undef PRIM_SIZE
}

void WriteStruct(char *&a_buff, const CStructData &a_data);
void WriteArray(char *&a_buff, const CArrayData &a_data);
void WriteBuffer(char *&a_buff, const CBufferData &a_data);
void WritePrimArray(char *&a_buff, const APrimArrayDataBase &a_data);

inline void WriteData(char *&a_buff, const AData &a_data)
{
#define WRITE_PRIM(name, type) \
    case DataType::name: \
        WritePrimitive(a_buff, static_cast<const CPrimData<type> &>(a_data).GetValue()); \
        break

    switch (a_data.Type())
    {
    WRITE_PRIM(SByte, int8_t);
    WRITE_PRIM(UByte, uint8_t);
    WRITE_PRIM(Short, short);
    WRITE_PRIM(UShort, ushort);
    WRITE_PRIM(Int, int);
    WRITE_PRIM(UInt, uint);
    WRITE_PRIM(Long, long long);
    WRITE_PRIM(ULong, ulong);
    WRITE_PRIM(Float, float);
    WRITE_PRIM(Double, double);
    WRITE_PRIM(Bool, bool);
    WRITE_PRIM(String, string);

    case DataType::Null:
        WriteValue(a_buff, NIL);
        break;

    case DataType::Struct:
        WriteStruct(a_buff, static_cast<const CStructData &>(a_data));
        break;

    case DataType::Array:
        WriteArray(a_buff, static_cast<const CArrayData &>(a_data));
        break;

    case DataType::Buffer:
        WriteBuffer(a_buff, static_cast<const CBufferData &>(a_data));
        break;

    case DataType::PrimArray:
        WritePrimArray(a_buff, static_cast<const APrimArrayDataBase &>(a_data));
        break;

    default:
        throw runtime_error("Unknown data type.");
    }

#undef WRITE_PRIM
}

AData::Ptr ReadString(const char *&a_buff, byte a_type, const CSerializationContext &a_context,
                      size_t a_length = SIZE_MAX);
AData::Ptr ReadArray(const char *&a_buff, byte a_type, const CSerializationContext &a_context,
                     size_t a_length = SIZE_MAX);
AData::Ptr ReadStruct(const char *&a_buff, byte a_type, const CSerializationContext &a_context,
                      size_t a_length = SIZE_MAX);
AData::Ptr ReadBuffer(const char *&a_buff, byte a_type, const CSerializationContext &a_context);

inline AData::Ptr ReadData(const char *&a_buff, const CSerializationContext &a_context)
{
    auto l_type = ReadValue<byte>(a_buff);

    if ((l_type & 0x80) == 0)
    {
        return MakePrim(l_type & ~POS_FIX_INT, a_context);
    }
    else if ((l_type & 0xe0) == NEG_FIX_INT)
    {
        auto l_val = reinterpret_cast<int8_t&>(l_type);
        return MakePrim(l_val, a_context);
    }
    else if ((l_type & 0xe0) == FIX_STR)
    {
        size_t l_len = l_type & ~FIX_STR;
        return ReadString(a_buff, FIX_STR, a_context, l_len);
    }
    else if ((l_type & 0xf0) == FIX_ARRAY)
    {
        size_t l_len = l_type & ~FIX_ARRAY;
        return ReadArray(a_buff, FIX_ARRAY, a_context, l_len);
    }
    else if ((l_type & 0xf0) == FIX_MAP)
    {
        size_t l_len = l_type & ~FIX_MAP;
        return ReadStruct(a_buff, FIX_MAP, a_context, l_len);
    }

    switch (l_type)
    {
    case NIL:
        return CNullData::Create(a_context);

    case TRUE:
        return MakePrim(true, a_context);
    case FALSE:
        return MakePrim(false, a_context);

    case UINT_8:
        return MakePrim(ReadValue<uint8_t>(a_buff), a_context);
    case INT_8:
        return MakePrim(ReadValue<int8_t>(a_buff), a_context);

    case UINT_16:
        return MakePrim(ReadValue<uint16_t>(a_buff), a_context);
    case INT_16:
        return MakePrim(ReadValue<int16_t>(a_buff), a_context);

    case UINT_32:
        return MakePrim(ReadValue<uint32_t>(a_buff), a_context);
    case INT_32:
        return MakePrim(ReadValue<int32_t>(a_buff), a_context);

    case UINT_64:
        return MakePrim(ReadValue<uint64_t>(a_buff), a_context);
    case INT_64:
        return MakePrim(ReadValue<int64_t>(a_buff), a_context);

    case FLOAT_32:
        return MakePrim(ReadValue<float>(a_buff), a_context);
    case FLOAT_64:
        return MakePrim(ReadValue<double>(a_buff), a_context);

    case STR_8:
    case STR_16:
    case STR_32:
        return ReadString(a_buff, l_type, a_context);

    case BIN_8:
    case BIN_16:
    case BIN_32:
        return ReadBuffer(a_buff, l_type, a_context);

    case ARR_16:
    case ARR_32:
        return ReadArray(a_buff, l_type, a_context);

    case MAP_16:
    case MAP_32:
        return ReadStruct(a_buff, l_type, a_context);

    default:
        throw runtime_error("Unsupported format type '" +
                            to_string(l_type) +
                            "'");
    }
}

inline AData::Ptr ReadString(const char *&a_buff, byte a_type,
                             const CSerializationContext &a_context,
                             size_t a_length)
{
    if (a_length == SIZE_MAX)
    {
        switch (a_type)
        {
        case STR_8:
            a_length = ReadValue<uint8_t>(a_buff);
            break;
        case STR_16:
            a_length = ReadValue<uint16_t>(a_buff);
            break;
        case STR_32:
            a_length = ReadValue<uint32_t>(a_buff);
            break;
        default:
            throw runtime_error("Unsupported string type '" +
                                to_string(a_type) +
                                "'");
        }
    }

    string l_val(a_buff, a_buff + a_length);
    a_buff += a_length;

    return MakePrim(move(l_val), a_context);
}

inline size_t CalcStructSize(const CStructData &a_data)
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
        len += CalcDataSize(*l_prop.second);
    }

    return len;
}

inline void WriteStruct(char *&a_buff, const CStructData &a_data)
{
    if (a_data.size() <= 0xf)
    {
        WriteValue<byte>(a_buff, FIX_MAP | byte(a_data.size()));
    }
    else if (a_data.size() <= 0xffff)
    {
        WriteValue(a_buff, MAP_16);
        WriteValue<uint16_t>(a_buff, a_data.size());
    }
    else
    {
        WriteValue(a_buff, MAP_32);
        WriteValue<uint32_t>(a_buff, a_data.size());
    }

    for (const CStructData::TProp &l_prop : a_data)
    {
        WritePrimitive(a_buff, l_prop.first);
        WriteData(a_buff, *l_prop.second);
    }
}

inline AData::Ptr ReadStruct(const char *&a_buff, byte a_type,
                             const CSerializationContext &a_context,
                             size_t a_length)
{
    if (a_length == SIZE_MAX)
    {
        switch (a_type)
        {
        case MAP_16:
            a_length = ReadValue<uint16_t>(a_buff);
            break;
        case MAP_32:
            a_length = ReadValue<uint32_t>(a_buff);
            break;
        default:
            throw runtime_error("Invalid map format '" +
                                to_string(a_type) +
                                "'");
        }
    }

    // Ok, so this is where things get tricky. This format doesn't have
    // structures as first class citizens, so we could either be reading
    // in a map, or a struct. It is not a struct if the key type is not a string
    // but if it is a string, then it's ambiguous. For the time being,
    // treat string-keyed maps as structs because that would be the common use case,
    // and a true map can always be serialized as an array of key-value structs
    vector<pair<AData::Ptr, AData::Ptr>> l_els;

    bool l_allStrings = true;
    for (size_t i = 0; i < a_length; ++i)
    {
        AData::Ptr l_key = ReadData(a_buff, a_context);
        AData::Ptr l_value = ReadData(a_buff, a_context);

        if (l_key->Type() != DataType::String)
        {
            l_allStrings = false;
        }

        l_els.emplace_back(move(l_key), move(l_value));
    }

    // Create a struct
    if (l_allStrings)
    {
        auto l_ret = CStructData::Create(a_context);

        for (auto &l_el : l_els)
        {
            l_ret->Add(move(static_cast<CPrimData<string>*>(l_el.first.get())->GetValue()),
                       move(l_el.second));
        }

        return move(l_ret);
    }
    // Create an array of structs
    else
    {
        auto l_ret = CArrayData::Create(a_context);

        for (auto &l_el : l_els)
        {
            auto l_p = CStructData::Create(a_context);
            l_p->Add("k", move(l_el.first));
            l_p->Add("v", move(l_el.second));
            l_ret->Add(move(l_p));
        }

        return move(l_ret);
    }
}

inline size_t CalcArraySize(const CArrayData &a_data)
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
        len += CalcDataSize(*l_data);
    }

    return len;
}

inline void WriteArray(char *&a_buff, const CArrayData &a_data)
{
    if (a_data.size() <= 0xf)
    {
        WriteValue<uint8_t>(a_buff, FIX_ARRAY | byte(a_data.size()));
    }
    else if (a_data.size() <= 0xffff)
    {
        WriteValue(a_buff, ARR_16);
        WriteValue<uint16_t>(a_buff, a_data.size());
    }
    else
    {
        WriteValue(a_buff, ARR_32);
        WriteValue<uint32_t>(a_buff, a_data.size());
    }

    for (const AData::Ptr &l_data : a_data)
    {
        WriteData(a_buff, *l_data);
    }
}

inline AData::Ptr ReadArray(const char *&a_buff, byte a_type,
                            const CSerializationContext &a_context,
                            size_t a_length)
{
    if (a_length == SIZE_MAX)
    {
        switch (a_type)
        {
        case ARR_16:
            a_length = ReadValue<uint16_t>(a_buff);
            break;
        case ARR_32:
            a_length = ReadValue<uint32_t>(a_buff);
            break;
        default:
            throw runtime_error("Unsupported array format '" +
                                to_string(a_type) +
                                "'");
        }
    }

    auto l_ret = CArrayData::Create(a_context);

    for (size_t i = 0; i < a_length; ++i)
    {
        l_ret->Add(ReadData(a_buff, a_context));
    }

    return move(l_ret);
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
        len += CalcSize(l_val);
    }

    return len;
}

template<typename T>
void WritePrimArrayImpl(char *&a_buff, const CPrimArrayData<T> &a_data)
{
    if (a_data.size() <= 0xf)
    {
        WriteValue<uint8_t>(a_buff, FIX_ARRAY | byte(a_data.size()));
    }
    else if (a_data.size() <= 0xffff)
    {
        WriteValue(a_buff, ARR_16);
        WriteValue<uint16_t>(a_buff, a_data.size());
    }
    else
    {
        WriteValue(a_buff, ARR_32);
        WriteValue<uint32_t>(a_buff, a_data.size());
    }

    for (const T &l_val : a_data)
    {
        WritePrimitive(a_buff, l_val);
    }
}

inline size_t CalcPrimArraySize(const APrimArrayDataBase &a_data)
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

inline void WritePrimArray(char *&a_buff, const APrimArrayDataBase &a_data)
{
#define WRITE_PRIM(name, type) \
    case DataType::name: \
        WritePrimArrayImpl(a_buff, static_cast<const CPrimArrayData<type> &>(a_data)); \
        break

    switch (a_data.InnerType())
    {
    WRITE_PRIM(SByte, signed char);
    WRITE_PRIM(UByte, unsigned char);
    WRITE_PRIM(Short, short);
    WRITE_PRIM(UShort, unsigned short);
    WRITE_PRIM(Int, int);
    WRITE_PRIM(UInt, unsigned int);
    WRITE_PRIM(Long, long long);
    WRITE_PRIM(ULong, unsigned long);
    WRITE_PRIM(Float, float);
    WRITE_PRIM(Double, double);
    WRITE_PRIM(Bool, bool);
    WRITE_PRIM(String, string);

    default:
        throw runtime_error("Unsupported primitive type.");
    }

#undef WRITE_PRIM
}

inline size_t CalcBufferSize(const CBufferData &a_data)
{
    return CalcRawSize(a_data.BufferSize());
}

inline void WriteBuffer(char *&a_buff, const CBufferData &a_data)
{
    if (a_data.BufferSize() <= 0xff)
    {
        WriteValue(a_buff, BIN_8);
        WriteValue<uint8_t>(a_buff, a_data.BufferSize());
    }
    else if (a_data.BufferSize() <= 0xffff)
    {
        WriteValue(a_buff, BIN_16);
        WriteValue<uint16_t>(a_buff, a_data.BufferSize());
    }
    else if (a_data.BufferSize() <= 0xffffffff)
    {
        WriteValue(a_buff, BIN_32);
        WriteValue<uint32_t>(a_buff, a_data.BufferSize());
    }
    else
    {
        throw runtime_error("Cannot write a buffer larger than (2^32)-1");
    }

    WriteRawBytes(a_buff, a_data.GetBuffer().data(), a_data.BufferSize());
}

inline AData::Ptr ReadBuffer(const char *&a_buff, byte a_type,
                             const CSerializationContext &a_context)
{
    size_t l_length;
    switch (a_type)
    {
    case BIN_8:
        l_length = ReadValue<uint8_t>(a_buff);
        break;
    case BIN_16:
        l_length = ReadValue<uint16_t>(a_buff);
        break;
    case BIN_32:
        l_length = ReadValue<uint32_t>(a_buff);
        break;
    default:
        throw runtime_error("Unsupported bin type '" +
                            to_string(a_type) +
                            "'");
    }

    util::CBuffer l_buff(l_length);
    memcpy(l_buff.data(), a_buff, l_length);
    a_buff += l_length;

    return CBufferData::Ptr(new CBufferData(move(l_buff), a_context));
}

}

size_t CMsgPackSerializer::CalcSize(const AData &a_data) const
{
    return CalcDataSize(a_data);
}

size_t CMsgPackSerializer::SerializeInto(const AData &a_data, char *a_buffer, size_t a_bufferSize) const
{
    char *l_writeBuff = a_buffer;

    WriteData(l_writeBuff, a_data);

    size_t l_writeSize = l_writeBuff - a_buffer;

    assert(l_writeSize <= a_bufferSize);

    return l_writeSize;
}

string CMsgPackSerializer::SerializeData(const AData &a_data) const
{
    size_t l_writeSize = CalcSize(a_data);

    string l_ret(l_writeSize, '\0');

    assert(SerializeInto(a_data, &l_ret[0], l_writeSize));

    return move(l_ret);
}


AData::Ptr CMsgPackSerializer::DeserializeData(const char *a_buf, const char *a_endBuf) const
{
    AData::Ptr l_ret = ReadData(a_buf, CSerializationContext());

    assert(a_buf <= a_endBuf);

    return move(l_ret);
}

} }
