/*
 * data_buffer.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef DATA_BUFFER_H_
#define DATA_BUFFER_H_

#include "util/buffer.h"
#include "serialization/master.h"

#include "../dll_export.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API CDataBuffer
{
public:
	typedef std::unique_ptr<char[]> TPtr;

private:
	TPtr m_buff;
	size_t m_buffSize;

public:
	typedef char *iterator;
	typedef const char *const_iterator;

	CDataBuffer();
    CDataBuffer(TPtr a_data, size_t a_dataSize);
	CDataBuffer(size_t a_dataSize);
	CDataBuffer(char *data, size_t dataSize);

	CDataBuffer(CDataBuffer &&other);
	CDataBuffer &operator=(CDataBuffer &&other);

	util::CBuffer ToShared();

	size_t Size() const { return m_buffSize; }

	char *Data() { return m_buff.get(); }
	const char *Data() const { return m_buff.get(); }

	char operator[](size_t idx) const
	{
		return *(m_buff.get() + idx);
	}

	char &operator[](size_t idx)
	{
		return *(m_buff.get() + idx);
	}

	void Reset();
	void Reset(size_t a_buffSize);

	void UpdateSize(size_t a_buffSize);

	static CDataBuffer Copy(const char *a_data, size_t a_dataSize);

	/*
	 * Breaking coding conventions here to better align with STL
	 * conventions
	 */
	size_t size() const { return Size(); }
	iterator begin() { return m_buff.get(); }
	const_iterator begin() const { return m_buff.get(); }

	iterator end() { return m_buff.get() + m_buffSize; }
	const_iterator end() const { return m_buff.get() + m_buffSize; }

	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }

	char *data() { return m_buff.get(); }
	const char *data() const { return m_buff.get(); }

private:
	// Don't allow copying
	CDataBuffer(const CDataBuffer &) = delete;
	CDataBuffer &operator=(const CDataBuffer &) = delete;
};

} 

namespace serialization {

template<>
struct AXON_SERIALIZE_API CDeserializer<communication::CDataBuffer>
{
    static void Deserialize(const AData &a_data, communication::CDataBuffer &a_buff)
    {
        if (DataType::Buffer == a_data.Type())
        {
            size_t l_bufSize;
            auto l_upBuff = const_cast<CBufferData*>(static_cast<const CBufferData *>(&a_data))->GetBuffer().Release(l_bufSize);

            a_buff = communication::CDataBuffer(std::move(l_upBuff), l_bufSize);
        }
        else
        {
            std::string l_enc = a_data.ToString();

            a_buff.Reset(l_enc.size());

            std::copy(l_enc.begin(), l_enc.end(), a_buff.begin());
        }
    }
};


}


}




#endif /* DATA_BUFFER_H_ */

