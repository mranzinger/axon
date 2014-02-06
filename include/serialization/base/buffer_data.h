/*
 * File description: buffer_data.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef BUFFER_DATA_H_
#define BUFFER_DATA_H_

#include "a_data.h"

#include "util/buffer.h"
#include "util/base64.h"

namespace axon { namespace serialization {

class CBufferData
	: public AData
{
private:
	util::CBuffer m_buffer;
	bool m_compress;

public:
	typedef std::unique_ptr<CBufferData> Ptr;

	CBufferData()
		: CBufferData(util::CBuffer())
	{
	}
	CBufferData(util::CBuffer a_buffer)
		: CBufferData(std::move(a_buffer), CSerializationContext())
	{
	}
	CBufferData(util::CBuffer a_buffer, bool a_compress)
		: CBufferData(std::move(a_buffer), a_compress, CSerializationContext())
	{
	}
	CBufferData(util::CBuffer a_buffer, CSerializationContext a_context)
		: CBufferData(std::move(a_buffer), false, std::move(a_context))
	{
	}
	CBufferData(util::CBuffer a_buffer, bool a_compress, CSerializationContext a_context)
		: AData(DataType::Buffer, std::move(a_context)), m_buffer(std::move(a_buffer)),
		  m_compress(a_compress)
	{
	}

	util::CBuffer &GetBuffer() { return m_buffer; }
	const util::CBuffer &GetBuffer() const { return m_buffer; }

	size_t BufferSize() const { return m_buffer.Size(); }

	bool Compress() const { return m_compress; }
	void SetCompress(bool a_val) { m_compress = a_val; }

	void SetBuffer(util::CBuffer a_buffer)
	{
		m_buffer = std::move(a_buffer);
	}

	ADATA_CASTER_FN_NOT_IMPL(SByte, sbyte);
	ADATA_CASTER_FN_NOT_IMPL(UByte, ubyte);
	ADATA_CASTER_FN_NOT_IMPL(Short, int16_t);
	ADATA_CASTER_FN_NOT_IMPL(UShort, uint16_t);
	ADATA_CASTER_FN_NOT_IMPL(Int, int32_t);
	ADATA_CASTER_FN_NOT_IMPL(UInt, uint32_t);
	ADATA_CASTER_FN_NOT_IMPL(Long, int64_t);
	ADATA_CASTER_FN_NOT_IMPL(ULong, uint64_t);
	ADATA_CASTER_FN_NOT_IMPL(Float, float);
	ADATA_CASTER_FN_NOT_IMPL(Double, double);
	ADATA_CASTER_FN_NOT_IMPL(String, std::string);
	ADATA_CASTER_FN_NOT_IMPL(Bool, bool);

	virtual std::string ToJsonString(size_t a_spaceIndent) const
	{
		std::string l_ret = "\"";

		util::CBase64::Encode((const unsigned char*)m_buffer.Data(), m_buffer.Size(), l_ret);

		l_ret += "\"";

		return l_ret;
	}
};

} }


#endif /* BUFFER_DATA_H_ */
