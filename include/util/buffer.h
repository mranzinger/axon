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
	
    enum Ownership
	{
		Unowned,
		TakeOwnership
	};

private:
	size_t m_buffSize;
	TPtr m_buff;
    Ownership *m_ownership;

public:
	typedef char *iterator;
	typedef const char *const_iterator;


	CBuffer()
		: m_buffSize(0), m_ownership(nullptr)
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
	{
        Reset(a_bufSize, std::move(a_buf));
	}
    CBuffer(const CBuffer &a_other)
    {
        Reset(a_other.m_buffSize, a_other.m_buff);
    }
    
    CBuffer &operator=(CBuffer a_other)
    {
        swap(*this, a_other);
        return *this;
    }

    friend void swap(CBuffer &a, CBuffer &b)
    {
        using std::swap;

        swap(a.m_buff, b.m_buff);
        swap(a.m_buffSize, b.m_buffSize);
        swap(a.m_ownership, b.m_ownership);
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
		else
		{
            auto l_ownership = m_ownership = new Ownership(a_mode);

            m_buff.reset(a_buff, 
                    [l_ownership] (const char *a_ptr)
                    {
                        if (*l_ownership != Unowned)
                            delete[] a_ptr; 
                        delete l_ownership;
                    });
		}
	}
	void Reset(size_t a_bufSize, TPtr a_buf)
	{
		m_buffSize = a_bufSize;
		
        auto l_ownership = m_ownership = new Ownership(Unowned);
        
        // The capture of the lambda is what actually ties
        // the lifetime of a_buf and m_buff. All that really needs
        // to be done is to clean up the ownership pointer.
        m_buff.reset(a_buf.get(), 
            [l_ownership, a_buf] (const char *a_ptr)
            {
                delete l_ownership;
            });
	}

    std::unique_ptr<char[]> Release(size_t &a_bufSize)
    {
        if (m_buff)
        {
            if (!m_buff.unique())
                throw std::runtime_error("Unable to release this buffer because it is shared across multiple objects.");

            *m_ownership = Unowned;
            a_bufSize = m_buffSize;
            m_buffSize = 0;
            auto l_ret = std::unique_ptr<char[]>(m_buff.get());
            m_buff.reset();
            return std::move(l_ret);
        }
        else
        {
            a_bufSize = 0;
            return std::unique_ptr<char[]>();
        }
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
