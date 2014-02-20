/*
 * data_buffer.cpp
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#include "messaging/data_buffer.h"

#include <string.h>

using namespace std;

namespace axon { namespace communication {

CDataBuffer::CDataBuffer()
	: m_buffSize(0)
{

}

CDataBuffer::CDataBuffer(size_t a_dataSize)
{
	Reset(a_dataSize);
}

CDataBuffer::CDataBuffer( char *data, size_t dataSize )
	: m_buff(data), m_buffSize(dataSize)
{

}

CDataBuffer::CDataBuffer( CDataBuffer &&other )
{
	*this = std::move(other);
}

CDataBuffer & CDataBuffer::operator=( CDataBuffer &&other )
{
	// Prevent self-assignment
	if (this == &other)
		return *this;

	m_buff = std::move(other.m_buff);
	m_buffSize = std::move(other.m_buffSize);

	return *this;
}

util::CBuffer CDataBuffer::ToShared()
{
	util::CBuffer ret(m_buffSize, m_buff.release(), util::CBuffer::TakeOwnership);
	m_buffSize = 0;
	return ret;
}

void CDataBuffer::Reset()
{
	m_buff.reset();
	m_buffSize = 0;
}

void CDataBuffer::Reset(size_t a_buffSize)
{
	if (!a_buffSize)
	{
		Reset();
		return;
	}

	m_buff.reset(new char[a_buffSize]);
	m_buffSize = a_buffSize;
}

void CDataBuffer::UpdateSize(size_t a_buffSize)
{
	m_buffSize = a_buffSize;
}

CDataBuffer CDataBuffer::Copy(const char* a_data, size_t a_dataSize)
{
	CDataBuffer l_ret(a_dataSize);

	memcpy(l_ret.data(), a_data, a_dataSize);

	return std::move(l_ret);
}

} }


