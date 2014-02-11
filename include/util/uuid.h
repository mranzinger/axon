/*
 * File description: uuid.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef UUID_H_
#define UUID_H_

#include <algorithm>
#include <cstdint>
#include <random>

#include "string_convert.h"

namespace axon { namespace util {

struct uuid
{
public:
	uint8_t m_data[16];

public:
	typedef uint8_t value_type;
    typedef uint8_t& reference;
    typedef uint8_t const& const_reference;
    typedef uint8_t* iterator;
    typedef uint8_t const* const_iterator;
    typedef size_t size_type;
    typedef std::ptrdiff_t difference_type;

public:
    iterator begin() { return m_data; } /* throw() */
    const_iterator begin() const { return m_data; } /* throw() */
    iterator end() { return m_data+size(); } /* throw() */
    const_iterator end() const { return m_data+size(); } /* throw() */

    size_type size() const { return sizeof(m_data); } /* throw() */

    bool is_nil() const /* throw() */
    {
        for(size_t i=0; i<size(); i++) {
            if (m_data[i] != 0U) {
                return false;
            }
        }
        return true;
    }

    void swap(uuid& rhs) /* throw() */
    {
        std::swap_ranges(begin(), end(), rhs.begin());
    }
};

inline bool operator==(uuid const& lhs, uuid const& rhs) /* throw() */
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

inline bool operator!=(uuid const& lhs, uuid const& rhs) /* throw() */
{
    return !(lhs == rhs);
}

inline bool operator<(uuid const& lhs, uuid const& rhs) /* throw() */
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

inline bool operator>(uuid const& lhs, uuid const& rhs) /* throw() */
{
    return rhs < lhs;
}
inline bool operator<=(uuid const& lhs, uuid const& rhs) /* throw() */
{
    return !(rhs < lhs);
}

inline bool operator>=(uuid const& lhs, uuid const& rhs) /* throw() */
{
    return !(lhs < rhs);
}

inline void swap(uuid& lhs, uuid& rhs) /* throw() */
{
    lhs.swap(rhs);
}

inline size_t hash_value(const uuid &u) /* throw() */
{
    size_t seed = 0;
    for(uuid::const_iterator i=u.begin(); i != u.end(); ++i)
    {
        seed ^= static_cast<size_t>(*i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}

uuid make_uuid()
{
	using namespace std;

	uuid u;

	random_device l_rd;
	default_random_engine l_engine(l_rd());

	uniform_int_distribution<unsigned long> l_dist(
			numeric_limits<unsigned long>::min(),
			numeric_limits<unsigned long>::max());

	auto randomVal = l_dist(l_engine);

	int i = 0;
	for (uuid::iterator it = u.begin(), end = u.end(); it != end; ++it, ++i)
	{
		if (i == sizeof(unsigned long))
		{
			randomVal = l_dist(l_engine);
			i = 0;
		}

		*it = static_cast<uuid::value_type>((randomVal >> (i * 8)) & 0xFF);
	}

	// set variant
	// must be 0b10xxxxxx
	*(u.begin()+8) &= 0xBF;
	*(u.begin()+8) |= 0x80;

	// set version
	// must be 0b0100xxxx
	*(u.begin()+6) &= 0x4F; //0b01001111
	*(u.begin()+6) |= 0x40; //0b01000000

	return u;
}

std::string ToString(const uuid &a_uuid)
{
	return std::string(a_uuid.begin(), a_uuid.end());
}

void StringTo(const std::string &a_str, uuid &a_val)
{
	memcpy(a_val.begin(), a_str.c_str(), sizeof(uuid));
}

} }



#endif /* UUID_H_ */
