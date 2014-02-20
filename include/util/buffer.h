/*
 * File description: buffer.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <memory>
#include <stdexcept>

#include "array_deleter.h"

namespace axon { namespace util {



class CBuffer
{
public:
	typedef std::shared_ptr<char> TPtr;
	typedef CArrayDeleter<char> TDeleter;

private:
	size_t m_buffSize;
	TPtr m_buff;

public:
	typedef char *iterator;
	typedef const char *const_iterator;

	enum Ownership
	{
		Unowned,
		TakeOwnership
	};

	CBuffer()
		: m_buffSize(0)
	{

	}
	CBuffer(size_t a_bufSize)
		: CBuffer(a_bufSize, new char[a_bufSize], TakeOwnership)
	{
	}
	CBuffer(size_t a_bufSize, char *a_buff, Ownership a_mode)
	{
		Reset(a_bufSize, a_buff, a_mode);
	}
	CBuffer(size_t a_bufSize, TPtr a_buf)
		: m_buffSize(a_bufSize), m_buff(std::move(a_buf))
	{
	}

	void Reset()
	{
		Reset(0);
	}
	void Reset(size_t a_bufSize)
	{
		auto l_buf = a_bufSize ? new char[a_bufSize] : nullptr;

		try
		{
			Reset(a_bufSize, l_buf, TakeOwnership);
		}
		catch (...)
		{
			delete[] l_buf;
			throw;
		}
	}
	void Reset(size_t a_bufSize, char *a_buff, Ownership a_mode)
	{
		m_buffSize = a_bufSize;

		if (0 == a_bufSize || !a_buff)
		{
			m_buffSize = 0;
			m_buff.reset();
		}
		else if (Unowned == a_mode)
		{
			// If this object doesn't assume ownership of the buffer,
			// then supply a deleter that does nothing
			m_buff.reset(a_buff, [] (const char*) {});
		}
		else if (TakeOwnership == a_mode)
		{
			m_buff.reset(a_buff, TDeleter());
		}
		else
			throw std::runtime_error("Unknown ownership mode.");
	}
	void Reset(size_t a_bufSize, TPtr a_buf)
	{
		m_buffSize = a_bufSize;
		m_buff = std::move(a_buf);
	}

	const char &operator[](size_t a_off) const
	{
		return *(begin() + a_off);
	}
	char &operator[](size_t a_off)
	{
		return *(begin() + a_off);
	}



	size_t Size() const { return m_buffSize; }

	char *Data() { return m_buff.get(); }
	const char *Data() const { return m_buff.get(); }
	const TPtr &SPData() const { return m_buff; }

	char *At(size_t a_off)
	{
		return begin() + a_off;
	}
	const char *At(size_t a_off) const
	{
		return begin() + a_off;
	}

	CBuffer SP_At(size_t a_off)
	{
		TPtr l_buff = m_buff;
		TPtr l_ret(At(a_off), [l_buff] (const char *){});

		return CBuffer(Size() - a_off, std::move(l_ret));
	}
	const CBuffer SP_At(size_t a_off) const
	{
		return const_cast<CBuffer*>(this)->SP_At(a_off);
	}


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
};

} }



#endif /* BUFFER_H_ */
