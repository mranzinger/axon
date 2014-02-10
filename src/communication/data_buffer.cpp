/*
 * data_buffer.cpp
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#include "messaging/data_buffer.h"

namespace axon { namespace communication {

CDataBuffer::CDataBuffer()
	: m_buffSize(0)
{

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

} }


