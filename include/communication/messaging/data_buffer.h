/*
 * data_buffer.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef DATA_BUFFER_H_
#define DATA_BUFFER_H_

#include "util/buffer.h"

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

	void Reset(size_t a_buffSize);

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

} }



#endif /* DATA_BUFFER_H_ */
