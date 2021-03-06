/*
 * File description: axon_serializer.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include "serialization/format/axon_serializer.h"

#include <unordered_map>
#include <type_traits>

using namespace std;

typedef unsigned char byte;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef uint64_t ulong;

namespace axon { namespace serialization {

const uint MAGIC_NUMBER = 0xBADF00D;
const ushort VERSION = 1;

struct MasterContext
	: IDataContext
{
	typedef unique_ptr<MasterContext> Ptr;
	typedef uint NameRef;

	size_t StorageSize;

	unordered_map<string, size_t> NameMap;
	unordered_map<size_t, string> ReverseMap;
};

namespace {

size_t p_CalcSize(const AData &a_data, MasterContext &a_mc, DataType a_knownType = DataType::Unknown);
size_t p_CalcHeaderSize(const AData &a_data, MasterContext &a_mc);

void WriteHeader(char *&a_buff, const MasterContext &a_mc);
void WriteData(char *&a_buff, const AData &a_data, const MasterContext &a_mc, DataType a_knownType = DataType::Unknown);
AData::Ptr ReadData(const char *&a_buff, const MasterContext &a_mc, const CSerializationContext &a_context, DataType a_knownType = DataType::Unknown);
void ReadHeader(const char *&a_buff, MasterContext &a_mc);

inline size_t CalcEncodeSize(size_t a_size)
{
	size_t l_ret = 1;

	for (; a_size > 0x7f; ++l_ret, a_size >>= 7);

	return l_ret;
}

inline void EncodeSize(char *&a_buffer, size_t a_size)
{
	for (; a_size > 0x7f; ++a_buffer, a_size >>= 7)
	{
		*a_buffer = char(0x80 | (a_size & 0x7F));
	}

	*a_buffer++ = char(a_size);
}

inline void DecodeSize(const char *&a_buffer, size_t &a_size)
{
	a_size = 0;

	size_t shift = 0;
	for (; *a_buffer & 0x80; ++a_buffer, shift += 7)
	{
		a_size |= size_t(*a_buffer & 0x7F) << shift;
	}

	a_size |= size_t(*a_buffer++) << shift;
}

inline size_t DecodeSize(const char *&a_buff)
{
	size_t l_ret;
	DecodeSize(a_buff, l_ret);
	return l_ret;
}

template<typename T>
size_t CalcValueSizeImpl(const typename enable_if<is_trivial<T>::value, T>::type &)
{
	return sizeof(T);
}

inline size_t CalcValueSize(const string &a_val)
{
	return CalcEncodeSize(a_val.size()) + a_val.size();
}

template<typename T>
size_t CalcValueSize(const T &a_val)
{
	return CalcValueSizeImpl<T>(a_val);
}

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

inline void WriteValue(char *&a_buff, const string &a_str)
{
	EncodeSize(a_buff, a_str.size()); // Write the size of the string

	memcpy(a_buff, a_str.data(), a_str.size());
	a_buff += a_str.size();
}

template<typename T>
void ReadValueImpl(const char *&a_buff, typename enable_if<is_trivial<T>::value, T>::type &a_val)
{
	memcpy(&a_val, a_buff, sizeof(T));
	a_buff += sizeof(T);
}

template<typename T>
void ReadValue(const char *&a_buff, T &a_val)
{
	ReadValueImpl<T>(a_buff, a_val);
}

inline void ReadValue(const char *&a_buff, string &a_str)
{
	size_t l_size;
	DecodeSize(a_buff, l_size);

	a_str = string(l_size, '\0');

	memcpy(const_cast<char*>(a_str.data()), a_buff, l_size);

	a_buff += l_size;
}

template<typename T>
T ReadValue(const char *&a_buff)
{
	T l_ret;
	ReadValue(a_buff, l_ret);
	return move(l_ret);
}

}

size_t CAxonSerializer::CalcSize(const AData& a_data) const
{
	MasterContext::Ptr l_master(new MasterContext);

	size_t l_dataSize = p_CalcSize(a_data, *l_master);
	l_dataSize += p_CalcHeaderSize(a_data, *l_master);

	l_master->StorageSize = l_dataSize;
	a_data.SetDataContext(move(l_master));

	return l_dataSize;
}

size_t CAxonSerializer::SerializeInto(const AData& a_data,
		char* a_buffer, size_t a_bufferSize) const
{
	size_t l_writeSize = p_EstablishSize(a_data);

	if (l_writeSize > a_bufferSize)
		return l_writeSize;

	const MasterContext &l_mc = *static_cast<MasterContext*>(a_data.GetDataContext());

	char *l_write = a_buffer;

	WriteHeader(l_write, l_mc);
	WriteData(l_write, a_data, l_mc);

	if ((l_write - a_buffer) != l_writeSize)
		throw runtime_error("The serialized data size did not match the calculated size.");

	return l_writeSize;
}

AData::Ptr CAxonSerializer::DeserializeData(
		const char* a_buf, const char* a_endBuf) const
{
	MasterContext l_mc;
	ReadHeader(a_buf, l_mc);

	AData::Ptr l_ret = ReadData(a_buf, l_mc, CSerializationContext());

	if (a_buf > a_endBuf)
		throw runtime_error("The specified byte stream was invalid. A data corruption has occurred.");

	return move(l_ret);
}

std::string CAxonSerializer::SerializeData(const AData& a_data) const
{
	size_t l_writeSize = p_EstablishSize(a_data);

	std::string l_ret(l_writeSize, '\0');

	if (SerializeInto(a_data, &l_ret[0], l_writeSize)
			!= l_writeSize)
	{
		throw runtime_error("Internal Error. Did not serialize properly.");
	}

	return move(l_ret);
}



size_t CAxonSerializer::p_EstablishSize(const AData& a_data) const
{
	auto l_cxt = dynamic_cast<MasterContext*>(a_data.GetDataContext());

	if (!l_cxt)
	{
		// Calculating the size of the data will establish the context
		return CalcSize(a_data);
	}
	else
	{
		return l_cxt->StorageSize;
	}
}

namespace {

inline size_t CalcBufferSize(const CBufferData &a_data)
{
	size_t l_size = 0;

	l_size += CalcEncodeSize(0); // Size of compressed buffer,
								 // which isn't currently supported, so store 0
	l_size += CalcEncodeSize(a_data.BufferSize()); // Size of buffer
	l_size += a_data.BufferSize(); // Buffer

	return l_size;
}

inline void WriteBuffer(char *&a_buff, const CBufferData &a_data)
{
	EncodeSize(a_buff, 0); // Size of compressed buffer
	EncodeSize(a_buff, a_data.BufferSize());

	memcpy(a_buff, a_data.GetBuffer().Data(), a_data.BufferSize());
	a_buff += a_data.BufferSize();
}

inline AData::Ptr ReadBuffer(const char *&a_buff, const CSerializationContext &a_context)
{
	size_t l_compSize = DecodeSize(a_buff);

	if (0 != l_compSize)
		throw runtime_error("Buffer decompression not supported.");

	size_t l_buffSize = DecodeSize(a_buff);

	util::CBuffer l_buff(l_buffSize);
	memcpy(l_buff.Data(), a_buff, l_buffSize);
	a_buff += l_buffSize;

	return CBufferData::Ptr(new CBufferData(move(l_buff), a_context));
}

inline size_t p_CalcStructSize(const CStructData &a_data, MasterContext &a_mc)
{
	size_t l_size = 0;

	l_size += sizeof(ubyte); // Write Mode (Currently only plain supported)
	l_size += CalcEncodeSize(a_data.size()); // Number of props

	for (const auto &l_prop : a_data)
	{
		// Add the property name to the name table
		auto l_iter = a_mc.NameMap.find(l_prop.first);
		if (l_iter == a_mc.NameMap.end())
		{
			l_iter = a_mc.NameMap.emplace(l_prop.first, a_mc.NameMap.size()).first;
		}

		l_size += CalcEncodeSize(l_iter->second); // Name reference
		l_size += p_CalcSize(*l_prop.second, a_mc); // Child value
	}

	return l_size;
}

inline void WriteStruct(char *&a_buff, const CStructData &a_data, const MasterContext &a_mc)
{
	WriteValue(a_buff, byte(0)); // Write Mode (Plain)
	EncodeSize(a_buff, a_data.size());

	for (const auto &l_prop : a_data)
	{
		EncodeSize(a_buff, a_mc.NameMap.find(l_prop.first)->second);
		WriteData(a_buff, *l_prop.second, a_mc);
	}
}

inline AData::Ptr ReadStruct(const char *&a_buff, const MasterContext &a_mc, const CSerializationContext &a_context)
{
	if (0 != ReadValue<byte>(a_buff))
		throw runtime_error("Unsupported struct format.");

	CStructData::Ptr l_ret(new CStructData(a_context));

	size_t l_numProps = DecodeSize(a_buff);

	for (size_t i = 0; i < l_numProps; ++i)
	{
		size_t l_nameId = DecodeSize(a_buff);

		auto l_nameIter = a_mc.ReverseMap.find(l_nameId);

		if (l_nameIter == a_mc.ReverseMap.end())
			throw runtime_error("Unknown property name.");

		AData::Ptr l_child = ReadData(a_buff, a_mc, a_context);

		l_ret->Add(l_nameIter->second, move(l_child));
	}

	return move(l_ret);
}

inline size_t p_CalcArraySize(const CArrayData &a_data, MasterContext &a_mc)
{
	size_t l_size = 0;

	l_size += sizeof(byte); // Write Mode (Currently only plain supported)
	l_size += CalcEncodeSize(a_data.size()); // Number of children

	for (const auto &l_val : a_data)
	{
		l_size += p_CalcSize(*l_val, a_mc);
	}

	return l_size;
}

inline void WriteArray(char *&a_buff, const CArrayData &a_data, const MasterContext &a_mc)
{
	WriteValue(a_buff, byte(0)); // Write Mode (Plain)
	EncodeSize(a_buff, a_data.size());

	for (const auto &l_val : a_data)
	{
		WriteData(a_buff, *l_val, a_mc);
	}
}

inline AData::Ptr ReadArray(const char *&a_buff, const MasterContext &a_mc,
                            const CSerializationContext &a_context)
{
	if (0 != ReadValue<byte>(a_buff))
		throw runtime_error("Invalid array format.");

	CArrayData::Ptr l_ret(new CArrayData(a_context));

	size_t l_arrSize = DecodeSize(a_buff);

	for (size_t i = 0; i < l_arrSize; ++i)
	{
		l_ret->Add(ReadData(a_buff, a_mc, a_context));
	}

	return move(l_ret);
}

template<typename T>
size_t p_CalcPrimArraySizeImpl(const CPrimArrayData<T> &a_data)
{
	size_t l_size = 0;

	l_size += sizeof(T) * a_data.size(); // Elements

	return l_size;
}

inline size_t p_CalcPrimArraySizeImpl(const CPrimArrayData<string> &a_data)
{
	size_t l_size = 0;

	for (const string &l_val : a_data)
	{
		l_size += CalcValueSize(l_val);
	}

	return l_size;
}

inline size_t CalcPrimArraySize(const APrimArrayDataBase &a_data)
{
#define CALC_SIZE(name, type) \
	case DataType::name: \
		l_size += p_CalcPrimArraySizeImpl(static_cast<const CPrimArrayData<type> &>(a_data)); \
		break

	size_t l_size = 0;

	l_size += sizeof(byte); // Write Mode
	l_size += sizeof(byte); // Inner Type
	l_size += CalcEncodeSize(a_data.Size()); // Number of elements

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

	return l_size;

#undef CALC_SIZE
}

template<typename T>
void WritePrimArrayImpl(char *&a_buff, const CPrimArrayData<T> &a_data)
{
	// Straight copy the primitive values into the output buffer
	size_t l_size = sizeof(T) * a_data.size();
	memcpy(a_buff, a_data.Data(), l_size);
	a_buff += l_size;
}

// Need to specialize this because of the boneheaded decision to make vector<bool>
// a bitset
inline void WritePrimArrayImpl(char *&a_buff, const CPrimArrayData<bool> &a_data)
{
	for (bool l_b : a_data)
	{
		WriteValue(a_buff, l_b);
	}
}

inline void WritePrimArrayImpl(char *&a_buff, const CPrimArrayData<string> &a_data)
{
	for (const string &l_val : a_data)
	{
		WriteValue(a_buff, l_val);
	}
}

inline void WritePrimArray(char *&a_buff, const APrimArrayDataBase &a_data)
{
#define WRITE_PRIM(name, type) \
	case DataType::name: \
		WritePrimArrayImpl(a_buff, static_cast<const CPrimArrayData<type> &>(a_data)); \
		break

	WriteValue(a_buff, byte(0)); // Write Mode
	WriteValue(a_buff, (byte)a_data.InnerType());
	EncodeSize(a_buff, a_data.Size());

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

template<typename T>
void ReadPrimArrayImpl2(const char *&a_buff, CPrimArrayData<T> &a_data, size_t a_size)
{
	const T *l_t = reinterpret_cast<const T *>(a_buff);

	auto l_end = l_t + a_size;

	a_data.Import(l_t, l_end);

	a_buff = reinterpret_cast<const char *>(l_end);
}

inline void ReadPrimArrayImpl2(const char *&a_buff, CPrimArrayData<string> &a_data, size_t a_size)
{
	for (size_t i = 0; i < a_size; ++i)
	{
		string l_str;
		ReadValue(a_buff, l_str);

		a_data.Add(move(l_str));
	}
}

template<typename T>
AData::Ptr ReadPrimArrayImpl(const char *&a_buff, size_t a_size, const CSerializationContext &a_context)
{
	typename CPrimArrayData<T>::Ptr l_ret(new CPrimArrayData<T>(a_context));
	ReadPrimArrayImpl2(a_buff, *l_ret, a_size);
	return move(l_ret);
}

inline AData::Ptr ReadPrimArray(const char *&a_buff, const CSerializationContext &a_context)
{
	if (0 != ReadValue<byte>(a_buff))
		throw runtime_error("Unsupported primitive array write mode.");

	DataType l_inner = (DataType)ReadValue<byte>(a_buff);

	size_t l_numElems = DecodeSize(a_buff);


#define READ_PRIM(name, type) \
	case DataType::name: \
		return ReadPrimArrayImpl<type>(a_buff, l_numElems, a_context);

	switch (l_inner)
	{
	READ_PRIM(SByte, signed char);
	READ_PRIM(UByte, unsigned char);
	READ_PRIM(Short, short);
	READ_PRIM(UShort, unsigned short);
	READ_PRIM(Int, int);
	READ_PRIM(UInt, unsigned int);
	READ_PRIM(Long, long long);
	READ_PRIM(ULong, unsigned long);
	READ_PRIM(Float, float);
	READ_PRIM(Double, double);
	READ_PRIM(Bool, bool);
	READ_PRIM(String, string);

	}

	throw runtime_error("Unsupported primitive type.");

#undef READ_PRIM
}

inline size_t p_CalcSize(const AData& a_data, MasterContext& a_mc, DataType a_knownType)
{
#define PRIM_SIZE(name, type) \
	case DataType::name: \
		l_size += CalcValueSize(static_cast<const CPrimData<type> &>(a_data).GetValue()); \
		break

	size_t l_size = 0;

	// Only store the type if it is not previously stored
	if (a_knownType == DataType::Unknown)
	{
		l_size += sizeof(byte); // Data Type
		a_knownType = a_data.Type();
	}

	switch (a_knownType)
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
		// Nothing to store
		break;

	case DataType::Struct:
		l_size += p_CalcStructSize(static_cast<const CStructData &>(a_data), a_mc);
		break;

	case DataType::Array:
		l_size += p_CalcArraySize(static_cast<const CArrayData &>(a_data), a_mc);
		break;

	case DataType::Buffer:
		l_size += CalcBufferSize(static_cast<const CBufferData &>(a_data));
		break;

	case DataType::PrimArray:
		l_size += CalcPrimArraySize(static_cast<const APrimArrayDataBase &>(a_data));
		break;
	}

	return l_size;

#undef PRIM_SIZE
}

inline void WriteData(char*& a_buff, const AData& a_data, const MasterContext& a_mc, DataType a_knownType)
{
#define WRITE_PRIM(name, type) \
	case DataType::name: \
		WriteValue(a_buff, static_cast<const CPrimData<type> &>(a_data).GetValue()); \
		break

	if (a_knownType == DataType::Unknown)
	{
		a_knownType = a_data.Type();
		WriteValue(a_buff, (byte)a_knownType);
	}

	switch (a_knownType)
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
		// Nothing to write
		break;

	case DataType::Struct:
		WriteStruct(a_buff, static_cast<const CStructData &>(a_data), a_mc);
		break;

	case DataType::Array:
		WriteArray(a_buff, static_cast<const CArrayData &>(a_data), a_mc);
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

inline AData::Ptr ReadData(const char*& a_buff, const MasterContext& a_mc, const CSerializationContext &a_context, DataType a_knownType)
{
#define READ_PRIM(name, type) \
	case DataType::name: \
		return MakePrim(ReadValue<type>(a_buff), a_context)

	if (a_knownType == DataType::Unknown)
	{
		a_knownType = (DataType)ReadValue<byte>(a_buff);
	}

	switch (a_knownType)
	{
	READ_PRIM(SByte, byte);
	READ_PRIM(UByte, byte);
	READ_PRIM(Short, short);
	READ_PRIM(UShort, ushort);
	READ_PRIM(Int, int);
	READ_PRIM(UInt, uint);
	READ_PRIM(Long, long long);
	READ_PRIM(ULong, ulong);
	READ_PRIM(Float, float);
	READ_PRIM(Double, double);
	READ_PRIM(Bool, bool);
	READ_PRIM(String, string);

	case DataType::Null:
		return CNullData::Create(a_context);

	case DataType::Struct:
		return ReadStruct(a_buff, a_mc, a_context);

	case DataType::Array:
		return ReadArray(a_buff, a_mc, a_context);

	case DataType::Buffer:
		return ReadBuffer(a_buff, a_context);

	case DataType::PrimArray:
		return ReadPrimArray(a_buff, a_context);
	}

	throw runtime_error("Unsupported data type.");

#undef READ_PRIM
}

inline size_t p_CalcHeaderSize(const AData& a_data, MasterContext& a_mc)
{
	size_t l_size = 0;

	l_size += sizeof(MAGIC_NUMBER); // Magic Number
	l_size += sizeof(VERSION); // Version
	l_size += CalcEncodeSize(a_mc.NameMap.size()); // Name table size

	for (const auto &l_nameRec : a_mc.NameMap)
	{
		l_size += CalcValueSize(l_nameRec.first);
		// Don't need to store the key because the names
		// will be written out in monotonically increasing order
	}

	return l_size;
}

inline void WriteHeader(char*& a_buff, const MasterContext& a_mc)
{
	typedef pair<size_t, const string*> record;

	WriteValue(a_buff, MAGIC_NUMBER);
	WriteValue(a_buff, VERSION);

	EncodeSize(a_buff, a_mc.NameMap.size());

	if (a_mc.NameMap.empty())
		return;

	vector<record> l_sorted;
	l_sorted.reserve(a_mc.NameMap.size());
	for (const auto &l_pair : a_mc.NameMap)
	{
		l_sorted.push_back(record(l_pair.second, &l_pair.first));
	}

	sort(l_sorted.begin(), l_sorted.end(),
			[] (const record &a, const record &b)
			{
				return a.first < b.first;
			});

	if (l_sorted.front().first != 0)
		throw runtime_error("Invalid first record.");

	for (const record &l_rec : l_sorted)
	{
		WriteValue(a_buff, *l_rec.second);
	}
}

inline void ReadHeader(const char*& a_buff, MasterContext& a_mc)
{
	if (ReadValue<remove_const<decltype(MAGIC_NUMBER)>::type>(a_buff) != MAGIC_NUMBER)
		throw runtime_error("The specified binary stream is not valid.");

	if (ReadValue<remove_const<decltype(VERSION)>::type>(a_buff) != 1)
		throw runtime_error("Only Version 1 byte streams are currently supported.");

	size_t l_tableSize = DecodeSize(a_buff);

	for (size_t i = 0; i < l_tableSize; ++i)
	{
		string l_key;
		ReadValue(a_buff, l_key);

		a_mc.ReverseMap.emplace(i, move(l_key));
	}
}

}



}
}


