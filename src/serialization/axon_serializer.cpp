/*
 * File description: axon_serializer.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
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

	unordered_map<string, NameRef> NameMap;
};

size_t p_CalcSize(const AData &a_data, MasterContext &a_mc, DataType a_knownType = DataType::Unknown);
size_t p_CalcHeaderSize(const AData &a_data, MasterContext &a_mc);

void WriteHeader(char *&a_buff, const MasterContext &a_mc);
void WriteData(char *&a_buff, const AData &a_data, const MasterContext &a_mc, DataType a_knownType = DataType::Unknown);

size_t CalcEncodeSize(size_t a_size)
{
	size_t l_ret = 1;

	for (; a_size > 0xFF; ++l_ret);

	return l_ret;
}

void EncodeSize(char *&a_buffer, size_t a_size)
{
	for (; a_size > 0x7f; ++a_buffer, a_size >>= 7)
	{
		*a_buffer = char(0x80 | (a_size & 0x7F));
	}

	*a_buffer++ = char(a_size);
}

void DecodeSize(const char *&a_buffer, size_t &a_size)
{
	a_size = 0;

	size_t shift = 0;
	for (; *a_buffer & 0x80; ++a_buffer, shift += 7)
	{
		a_size |= size_t(*a_buffer & 0x7F) << shift;
	}

	a_size |= size_t(*a_buffer++) << shift;
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

void WriteValue(char *&a_buff, const std::string &a_str)
{
	EncodeSize(a_buff, a_str.size()); // Write the size of the string

	memcpy(a_buff, a_str.data(), a_str.size());
	a_buff += a_str.size();
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

std::string CAxonSerializer::SerializeData(const AData& a_data) const
{
	size_t l_writeSize = p_EstablishSize(a_data);

	std::string l_ret(l_writeSize, '\0');

	if (SerializeInto(a_data, const_cast<char*>(l_ret.data()), l_writeSize)
			!= l_writeSize)
	{
		throw runtime_error("Internal Error. Did not serialize properly.");
	}

	return move(l_ret);
}

AData::Ptr CAxonSerializer::DeserializeData(
		const char* a_buf, const char* a_endBuf) const
{
	// TODO:
	throw runtime_error("Not Implemented.");
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

size_t p_CalcStringSize(const CPrimData<string> &a_data)
{
	size_t l_size = 0;

	l_size += CalcEncodeSize(a_data.GetValue().size()); // Size of string, encoded
	l_size += a_data.GetValue().size(); // String

	return l_size;
}

size_t p_CalcBufferSize(const CBufferData &a_data)
{
	size_t l_size = 0;

	l_size += CalcEncodeSize(0); // Size of compressed buffer,
								 // which isn't currently supported, so store 0
	l_size += CalcEncodeSize(a_data.BufferSize()); // Size of buffer
	l_size += a_data.BufferSize(); // Buffer

	return l_size;
}

void WriteBuffer(char *&a_buff, const CBufferData &a_data)
{
	EncodeSize(a_buff, 0); // Size of compressed buffer
	EncodeSize(a_buff, a_data.BufferSize());

	memcpy(a_buff, a_data.GetBuffer().Data(), a_data.BufferSize());
	a_buff += a_data.BufferSize();
}

size_t p_CalcStructSize(const CStructData &a_data, MasterContext &a_mc)
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

void WriteStruct(char *&a_buff, const CStructData &a_data, const MasterContext &a_mc)
{
	WriteValue(a_buff, byte(0)); // Write Mode (Plain)
	EncodeSize(a_buff, a_data.size());

	for (const auto &l_prop : a_data)
	{
		EncodeSize(a_buff, a_mc.NameMap.find(l_prop.first)->second);
		WriteData(a_buff, *l_prop.second, a_mc);
	}
}

size_t p_CalcArraySize(const CArrayData &a_data, MasterContext &a_mc)
{
	size_t l_size = 0;

	l_size += sizeof(ubyte); // Write Mode (Currently only plain supported)
	l_size += CalcEncodeSize(a_data.size()); // Number of children

	for (const auto &l_val : a_data)
	{
		l_size += p_CalcSize(*l_val, a_mc);
	}

	return l_size;
}

void WriteArray(char *&a_buff, const CArrayData &a_data, const MasterContext &a_mc)
{
	WriteValue(a_buff, byte(0)); // Write Mode (Plain)
	EncodeSize(a_buff, a_data.size());

	for (const auto &l_val : a_data)
	{
		WriteData(a_buff, *l_val, a_mc);
	}
}

size_t p_CalcSize(const AData& a_data, MasterContext& a_mc, DataType a_knownType)
{
#define PRIM_SIZE(name, type) \
	case DataType::name: \
		l_size += sizeof(type); \
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

	case DataType::Null:
		// Nothing to store
		break;

	case DataType::String:
		l_size += p_CalcStringSize(static_cast<const CPrimData<string> &>(a_data));
		break;

	case DataType::Struct:
		l_size += p_CalcStructSize(static_cast<const CStructData &>(a_data), a_mc);
		break;

	case DataType::Array:
		l_size += p_CalcArraySize(static_cast<const CArrayData &>(a_data), a_mc);
		break;

	case DataType::Buffer:
		l_size += p_CalcBufferSize(static_cast<const CBufferData &>(a_data));
		break;
	}

	return l_size;

#undef PRIM_SIZE
}

void WriteData(char*& a_buff, const AData& a_data, const MasterContext& a_mc, DataType a_knownType)
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
	WRITE_PRIM(SByte, byte);
	WRITE_PRIM(UByte, byte);
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
	}
}

size_t p_CalcHeaderSize(const AData& a_data, MasterContext& a_mc)
{
	size_t l_size = 0;

	l_size += sizeof(MAGIC_NUMBER); // Magic Number
	l_size += sizeof(VERSION); // Version
	l_size += CalcEncodeSize(a_mc.NameMap.size()); // Name table size

	for (const auto &l_nameRec : a_mc.NameMap)
	{
		l_size += CalcEncodeSize(l_nameRec.first.size()); // Name Length
		l_size += l_nameRec.first.size(); // Name
		// Don't need to store the key because the names
		// will be written out in monotonically increasing order
	}

	return l_size;
}

void WriteHeader(char*& a_buff, const MasterContext& a_mc)
{
	typedef pair<uint, const string*> record;

	WriteValue(a_buff, MAGIC_NUMBER);
	WriteValue(a_buff, VERSION);

	EncodeSize(a_buff, a_mc.NameMap.size());

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



}
}


