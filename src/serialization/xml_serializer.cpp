
/*
 * File description: xml_serializer.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include "format/xml_serializer.h"

#include <sstream>

#include "pugixml/pugixml.hpp"

using namespace std;
using namespace pugi;

namespace axon { namespace serialization {

void WriteValue(xml_node a_node, const AData &a_data);
AData::Ptr ReadValue(xml_node a_node, const CSerializationContext &a_context);

std::string CXmlSerializer::SerializeData(const AData& a_data) const
{
	xml_document l_doc;

	auto l_root = l_doc.append_child("Root");
	WriteValue(l_root, a_data);

	stringstream l_ss;
	l_doc.save(l_ss);

	return l_ss.str();
}

AData::Ptr CXmlSerializer::DeserializeData(const char* a_buf, const char* a_endBuf) const
{
	if (!a_buf || a_endBuf <= a_buf)
		throw std::runtime_error("Invalid XML document.");

	xml_document l_doc;
	l_doc.load_buffer(a_buf, a_endBuf - a_buf);

	return ReadValue(l_doc.child("Root"), CSerializationContext());
}

void WriteStruct(xml_node a_node, const CStructData &a_data)
{
	a_node.append_attribute("_t").set_value("Struct");

	for (const auto &prop : a_data)
	{
		auto l_child = a_node.append_child("P");
		l_child.append_attribute("_n").set_value(prop.first.c_str());
		WriteValue(l_child, *prop.second);
	}
}

void WriteArray(xml_node a_node, const CArrayData &a_data)
{
	a_node.append_attribute("_t").set_value("Array");

	for (const auto &l_val : a_data)
	{
		auto l_child = a_node.append_child("V");
		WriteValue(l_child, *l_val);
	}
}

void WriteBuffer(xml_node a_node, const CBufferData &a_data)
{
	a_node.append_attribute("_t").set_value("Buffer");

	std::string l_encoded = util::CBase64::Encode(a_data.GetBuffer().Data(), a_data.GetBuffer().Size());

	a_node.append_attribute("_enclen").set_value((uint)l_encoded.size());
	a_node.append_attribute("_rawlen").set_value((uint)a_data.GetBuffer().Size());
	a_node.append_attribute("_compressed").set_value(false);

	a_node.append_child(node_pcdata).set_value(l_encoded.c_str());
}

void WriteValue(xml_node a_node, const AData& a_data)
{
#define VALSET(datatype) \
	case DataType::datatype: \
		a_node.append_attribute("_t").set_value(#datatype); \
		a_node.append_attribute("val").set_value(a_data.ToString().c_str()); \
		break

	switch (a_data.Type())
	{
	VALSET(SByte);
	VALSET(UByte);
	VALSET(Short);
	VALSET(UShort);
	VALSET(Int);
	VALSET(UInt);
	VALSET(Long);
	VALSET(ULong);
	VALSET(Float);
	VALSET(Double);
	VALSET(Bool);
	VALSET(String);

	case DataType::Null:
		a_node.append_attribute("_t").set_value("Null");
		break;

	case DataType::Struct:
		WriteStruct(a_node, static_cast<const CStructData &>(a_data));
		break;

	case DataType::Array:
		WriteArray(a_node, static_cast<const CArrayData &>(a_data));
		break;

	case DataType::Buffer:
		WriteBuffer(a_node, static_cast<const CBufferData &>(a_data));
		break;

	default:
		throw std::runtime_error("Unsupported data type.");
	}



#undef VALSET
}

AData::Ptr ReadStruct(xml_node a_node, const CSerializationContext &a_context)
{
	CStructData::Ptr l_ret(new CStructData(a_context));

	for (xml_node l_child : a_node)
	{
		string l_name = l_child.attribute("_n").value();
		AData::Ptr l_val = ReadValue(l_child, a_context);

		l_ret->Add(move(l_name), move(l_val));
	}

	return move(l_ret);
}

AData::Ptr ReadArray(xml_node a_node, const CSerializationContext &a_context)
{
	CArrayData::Ptr l_ret(new CArrayData(a_context));

	for (xml_node l_child : a_node)
	{
		l_ret->Add(ReadValue(l_child, a_context));
	}

	return move(l_ret);
}

AData::Ptr ReadBuffer(xml_node a_node, const CSerializationContext &a_context)
{
	auto l_encLen = cast_to<size_t>(a_node.attribute("_enclen").value());
	auto l_rawLen = cast_to<size_t>(a_node.attribute("_rawlen").value());
	auto l_compressed = a_node.attribute("_compressed").as_bool();

	if (l_compressed)
		throw std::runtime_error("Compressed buffers not currently supported.");

	util::CBuffer l_buff(l_rawLen);

	size_t l_decLen = util::CBase64::Decode(a_node.child_value(), l_encLen, (unsigned char*)l_buff.Data(), l_buff.Size());

	if (l_decLen != l_rawLen)
		throw std::runtime_error("The Base 64 encoded value was corrupt.");

	return AData::Ptr(new CBufferData(std::move(l_buff), l_compressed, a_context));
}

AData::Ptr ReadValue(xml_node a_node, const CSerializationContext& a_context)
{
	const char *l_type = a_node.attribute("_t").as_string();

	if (0 == strcmp(l_type, "SByte"))
		return MakePrim(a_node.attribute("val").as_int(), a_context);
	if (0 == strcmp(l_type, "UByte"))
		return MakePrim(a_node.attribute("val").as_int(), a_context);
	if (0 == strcmp(l_type, "Short"))
		return MakePrim(a_node.attribute("val").as_int(), a_context);
	if (0 == strcmp(l_type, "UShort"))
		return MakePrim(a_node.attribute("val").as_int(), a_context);
	if (0 == strcmp(l_type, "Int"))
		return MakePrim(a_node.attribute("val").as_int(), a_context);
	if (0 == strcmp(l_type, "UInt"))
		return MakePrim(a_node.attribute("val").as_uint(), a_context);
	if (0 == strcmp(l_type, "Long"))
		return MakePrim(cast_to<long long>(a_node.attribute("val").value()), a_context);
	if (0 == strcmp(l_type, "ULong"))
		return MakePrim(cast_to<unsigned long long>(a_node.attribute("val").value()), a_context);
	if (0 == strcmp(l_type, "Float"))
		return MakePrim(a_node.attribute("val").as_float(), a_context);
	if (0 == strcmp(l_type, "Double"))
		return MakePrim(a_node.attribute("val").as_double(), a_context);
	if (0 == strcmp(l_type, "Bool"))
		return MakePrim(a_node.attribute("val").as_bool(), a_context);
	if (0 == strcmp(l_type, "String"))
		return MakePrim(a_node.attribute("val").value(), a_context);
	if (0 == strcmp(l_type, "Null"))
		return CNullData::Create(a_context);
	if (0 == strcmp(l_type, "Struct"))
		return ReadStruct(a_node, a_context);
	if (0 == strcmp(l_type, "Array"))
		return ReadArray(a_node, a_context);
	if (0 == strcmp(l_type, "Buffer"))
		return ReadBuffer(a_node, a_context);

	throw std::runtime_error("Unsupported data type.");
}

}
}


