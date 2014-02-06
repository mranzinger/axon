/*
 * File description: json_serializer.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include "format/json_serializer.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

namespace axon { namespace serialization {

void WriteValue(rapidjson::Document &a_doc, rapidjson::Value &a_jsVal, const AData &a_data);
AData::Ptr ReadValue(const rapidjson::Document &a_doc, const rapidjson::Value &a_jsVal, const CSerializationContext &a_context);


std::string CJsonSerializer::SerializeData(
		const AData& a_data) const
{
	rapidjson::Document l_doc;
	WriteValue(l_doc, l_doc, a_data);

	rapidjson::StringBuffer l_buff;
	rapidjson::PrettyWriter<decltype(l_buff)> l_writer(l_buff);

	l_doc.Accept(l_writer);

	return std::string(l_buff.GetString(), l_buff.Size());
}

AData::Ptr CJsonSerializer::DeserializeData(
		const char* a_buf, const char* a_endBuf) const
{
	if (!a_buf || a_endBuf <= a_buf)
		throw std::runtime_error("Invalid JSON document.");

	rapidjson::Document l_doc;

	std::string l_buffStr;

	if (*(a_endBuf -1) == '\0')
	{
		l_doc.Parse<0>(a_buf);
	}
	else
	{
		l_buffStr = std::string(a_buf, a_endBuf);
		l_doc.ParseInsitu<0>(const_cast<char*>(l_buffStr.c_str()));
	}

	return ReadValue(l_doc, l_doc, CSerializationContext());
}

void WriteString(rapidjson::Document &a_doc, rapidjson::Value &a_jsVal, const CPrimData<std::string> &a_data)
{
	a_jsVal.SetString(a_data.GetValue().data(), a_data.GetValue().size());
}

void WriteStruct(rapidjson::Document &a_doc, rapidjson::Value &a_jsVal, const CStructData &a_data)
{
	a_jsVal.SetObject();

	for (const auto &prop : a_data)
	{
		rapidjson::Value l_val;
		WriteValue(a_doc, l_val, *prop.second);

		a_jsVal.AddMember(prop.first.c_str(), l_val, a_doc.GetAllocator());
	}
}

void WriteArray(rapidjson::Document &a_doc, rapidjson::Value &a_jsVal, const CArrayData &a_data)
{
	a_jsVal.SetArray();

	for (const auto &val : a_data)
	{
		rapidjson::Value l_val;
		WriteValue(a_doc, l_val, *val);

		a_jsVal.PushBack(l_val, a_doc.GetAllocator());
	}
}

void WritePrimArray(rapidjson::Document &a_doc, rapidjson::Value &a_jsVal, const APrimArrayDataBase &a_data)
{
	a_jsVal.SetObject();
	a_jsVal.AddMember("_t", "_prim", a_doc.GetAllocator());
	a_jsVal.AddMember("_it", (int)a_data.InnerType(), a_doc.GetAllocator());

	auto l_reg = a_data.ToRegArray();

	rapidjson::Value l_arr(rapidjson::kArrayType);

	for (const auto &val : *l_reg)
	{
		rapidjson::Value l_val;
		WriteValue(a_doc, l_val, *val);

		l_arr.PushBack(l_val, a_doc.GetAllocator());
	}

	a_jsVal.AddMember("_v", l_arr, a_doc.GetAllocator());
}

void WriteBuffer(rapidjson::Document &a_doc, rapidjson::Value &a_jsVal, const CBufferData &a_data)
{
	a_jsVal.SetObject();

	a_jsVal.AddMember("_t", "_buffer", a_doc.GetAllocator());

	std::string l_encoded = util::CBase64::Encode(a_data.GetBuffer().Data(), a_data.GetBuffer().Size());

	a_jsVal.AddMember("_enclen", l_encoded.size(), a_doc.GetAllocator());
	a_jsVal.AddMember("_rawlen", a_data.GetBuffer().Size(), a_doc.GetAllocator());
	a_jsVal.AddMember("_compressed", false, a_doc.GetAllocator()); // TODO: Support compression

	rapidjson::Value l_arr(rapidjson::kArrayType);

	for (size_t i = 0; i < l_encoded.size(); i += 80)
	{
		size_t l_end = std::min(i + 80, l_encoded.size());

		std::string l_subVal(l_encoded.substr(i, l_end - i));

		rapidjson::Value l_val(rapidjson::kStringType);
		l_val.SetString(l_subVal.c_str(), l_subVal.size());

		l_arr.PushBack(l_val, a_doc.GetAllocator());
	}

	a_jsVal.AddMember("_data", l_arr, a_doc.GetAllocator());
}

void WriteValue(rapidjson::Document &a_doc, rapidjson::Value &a_jsVal, const AData &a_data)
{
#define VALSET(datatype, jsname, type) \
	case DataType::datatype: \
		a_jsVal.Set ## jsname(static_cast<const CPrimData<type> &>(a_data).GetValue()); \
		break

	switch (a_data.Type())
	{
	VALSET(SByte, Int, sbyte);
	VALSET(UByte, Int, ubyte);
	VALSET(Short, Int, short);
	VALSET(UShort, Int, unsigned short);
	VALSET(Int, Int, int);
	VALSET(UInt, Uint, unsigned int);
	VALSET(Long, Int64, long long);
	VALSET(ULong, Uint64, unsigned long long);
	VALSET(Float, Double, float);
	VALSET(Double, Double, double);
	VALSET(Bool, Bool, bool);

	case DataType::String:
		WriteString(a_doc, a_jsVal, static_cast<const CPrimData<std::string> &>(a_data));
		break;
	case DataType::Struct:
		WriteStruct(a_doc, a_jsVal, static_cast<const CStructData &>(a_data));
		break;
	case DataType::Array:
		WriteArray(a_doc, a_jsVal, static_cast<const CArrayData &>(a_data));
		break;
	case DataType::Buffer:
		WriteBuffer(a_doc, a_jsVal, static_cast<const CBufferData &>(a_data));
		break;
	case DataType::Null:
		a_jsVal.SetNull();
		break;
	case DataType::PrimArray:
		WritePrimArray(a_doc, a_jsVal, static_cast<const APrimArrayDataBase &>(a_data));
		break;

	default:
		throw std::runtime_error("Unsupported data type.");
	}



#undef VALSET
}

const rapidjson::Value *GetMemVal(const rapidjson::Value &a_jsVal, const char *a_name, bool a_force = true)
{
	for (auto iter = a_jsVal.MemberBegin(), end = a_jsVal.MemberEnd(); iter != end; ++iter)
	{
		if (0 == strcmp(iter->name.GetString(), a_name))
			return &iter->value;
	}

	if (a_force)
		throw std::runtime_error("The specified member does not exist.");

	return nullptr;
}

AData::Ptr ReadBuffer(const rapidjson::Document& a_doc, const rapidjson::Value& a_jsVal, const CSerializationContext &a_context)
{
	auto l_encLen = GetMemVal(a_jsVal, "_enclen")->GetUint64();
	auto l_rawLen = GetMemVal(a_jsVal, "_rawlen")->GetUint64();
	auto l_compressed = GetMemVal(a_jsVal, "_compressed")->GetBool();

	if (l_compressed)
		throw std::runtime_error("Compressed buffers not currently supported.");

	const rapidjson::Value &l_data = *GetMemVal(a_jsVal, "_data");

	std::unique_ptr<char[]> l_encBuff(new char[l_encLen]);

	char *l_encCurr = l_encBuff.get();
	for (size_t i = 0, end = l_data.Size(); i < end; ++i)
	{
		const rapidjson::Value &l_part = l_data[i];

		std::copy(l_part.GetString(), l_part.GetString() + l_part.GetStringLength(), l_encCurr);
		l_encCurr += l_part.GetStringLength();
	}

	util::CBuffer l_buff(l_rawLen);

	if (l_rawLen)
	{
		size_t l_decLen = util::CBase64::Decode(l_encBuff.get(), l_encLen, (unsigned char*)l_buff.Data(), l_buff.Size());

		if (l_decLen != l_rawLen)
			throw std::runtime_error("The Base 64 encoded value was corrupt.");
	}

	return AData::Ptr(new CBufferData(std::move(l_buff), l_compressed, a_context));
}

AData::Ptr ReadArray(const rapidjson::Document& a_doc, const rapidjson::Value& a_jsVal, const CSerializationContext &a_context)
{
	CArrayData::Ptr l_ret(new CArrayData(a_context));

	for (auto iter = a_jsVal.Begin(), end = a_jsVal.End(); iter != end; ++iter)
	{
		l_ret->Add(ReadValue(a_doc, *iter, a_context));
	}

	return std::move(l_ret);
}

template<typename T>
APrimArrayDataBase::Ptr ReadPrimArrayImpl(const rapidjson::Document &a_doc, const rapidjson::Value &a_jsVal, const CSerializationContext &a_context)
{
	typename CPrimArrayData<T>::Ptr l_ret(new CPrimArrayData<T>(a_context));

	auto l_arr = GetMemVal(a_jsVal, "_v");

	for (auto iter = l_arr->Begin(), end = l_arr->End(); iter != end; ++iter)
	{
		switch (iter->GetType())
		{
		case rapidjson::kFalseType:
			l_ret->Add(cast_to<T>(false));
			break;
		case rapidjson::kTrueType:
			l_ret->Add(cast_to<T>(true));
			break;
		case rapidjson::kNumberType:
			l_ret->Add(cast_to<T>(iter->GetDouble()));
			break;
		case rapidjson::kStringType:
			l_ret->Add(cast_to<T>(iter->GetString()));
			break;
		default:
			throw std::runtime_error("Unsupported primitive type.");
		}
	}

	return std::move(l_ret);
}

AData::Ptr ReadPrimArray(const rapidjson::Document &a_doc, const rapidjson::Value &a_jsVal, const CSerializationContext &a_context)
{
#define READ_PRIM(name, type) \
	case DataType::name: \
		return ReadPrimArrayImpl<type>(a_doc, a_jsVal, a_context)

	DataType l_innerType = (DataType)GetMemVal(a_jsVal, "_it")->GetInt();

	switch (l_innerType)
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
	READ_PRIM(String, std::string);
	}

#undef READ_PRIM
}

AData::Ptr ReadObject(const rapidjson::Document& a_doc, const rapidjson::Value& a_jsVal, const CSerializationContext &a_context)
{
	const rapidjson::Value *l_typeVal = GetMemVal(a_jsVal, "_t", false);

	if (l_typeVal)
	{
		if (0 == strcmp(l_typeVal->GetString(), "_buffer"))
		{
			return ReadBuffer(a_doc, a_jsVal, a_context);
		}
		else if (0 == strcmp(l_typeVal->GetString(), "_prim"))
		{
			return ReadPrimArray(a_doc, a_jsVal, a_context);
		}
	}

	CStructData::Ptr l_ret(new CStructData(a_context));

	for (auto iter = a_jsVal.MemberBegin(), end = a_jsVal.MemberEnd(); iter != end; ++iter)
	{
		AData::Ptr l_val = ReadValue(a_doc, iter->value, a_context);

		l_ret->Add(std::string(iter->name.GetString(), iter->name.GetStringLength()), std::move(l_val));
	}

	return std::move(l_ret);
}



AData::Ptr ReadValue(const rapidjson::Document& a_doc, const rapidjson::Value& a_jsVal, const CSerializationContext &a_context)
{
	switch (a_jsVal.GetType())
	{
	case rapidjson::kNullType:
		return CNullData::Create(a_context);
	case rapidjson::kTrueType:
		return MakePrim(true, a_context);
	case rapidjson::kFalseType:
		return MakePrim(false, a_context);

	case rapidjson::kNumberType:
		if (a_jsVal.IsInt())
			return MakePrim(a_jsVal.GetInt(), a_context);
		if (a_jsVal.IsUint())
			return MakePrim(a_jsVal.GetUint(), a_context);
		if (a_jsVal.IsInt64())
			return MakePrim(a_jsVal.GetInt64(), a_context);
		if (a_jsVal.IsUint64())
			return MakePrim(a_jsVal.GetUint64(), a_context);
		if (a_jsVal.IsDouble())
			return MakePrim(a_jsVal.GetDouble(), a_context);

		throw std::runtime_error("Unknown number type.");

	case rapidjson::kStringType:
		return MakePrim(std::string(a_jsVal.GetString(), a_jsVal.GetStringLength()), a_context);

	case rapidjson::kObjectType:
		return ReadObject(a_doc, a_jsVal, a_context);
	case rapidjson::kArrayType:
		return ReadArray(a_doc, a_jsVal, a_context);
	}

	throw std::runtime_error("Unknown JSON type.");
}

}
}


