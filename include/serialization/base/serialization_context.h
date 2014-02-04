/*
 * File description: serialization_context.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef SERIALIZATION_CONTEXT_H_
#define SERIALIZATION_CONTEXT_H_

#include <stdint.h>

#include "detail/serialization_context_impl.h"

namespace axon { namespace serialization {

enum class SerializationFlags : uint32_t
{
	None = 0,
	Shareable = 0x1,
	Compress = 0x2
};

class CSerializationContext
{
private:
	typedef detail::CSerializationContextImpl Impl;

	Impl *m_impl;
	mutable uint32_t m_flags = (uint32_t)SerializationFlags::None;

public:
	CSerializationContext()
		: m_impl(new Impl)
	{

	}
	CSerializationContext(const CSerializationContext &a_other)
		: m_impl(nullptr)
	{
		*this = a_other;
	}
	CSerializationContext(CSerializationContext &&a_other)
		: m_impl(nullptr)
	{
		*this = std::move(a_other);
	}
	~CSerializationContext()
	{
		if (m_impl)
			m_impl->RemoveRef();
	}

	CSerializationContext &operator=(const CSerializationContext &a_other)
	{
		Impl *l_other = a_other.m_impl;
		l_other->AddRef();

		if (m_impl)
			m_impl->RemoveRef();

		m_impl = l_other;
		return *this;
	}
	CSerializationContext &operator=(CSerializationContext &&a_other)
	{
		std::swap(m_impl, a_other.m_impl);
		return *this;
	}

	void ToggleFlags() const { }
	template<typename ...Flags>
	void ToggleFlags(SerializationFlags a_flags, Flags ...a_rest) const
	{
		m_flags ^= (uint32_t)a_flags;
		ToggleFlags(a_rest...);
	}

	bool IsSharedSet() const
	{
		return (m_flags & (uint32_t)SerializationFlags::Shareable);
	}
	bool IsCompressSet() const
	{
		return (m_flags & (uint32_t)SerializationFlags::Compress);
	}
};

template<SerializationFlags ...Flags>
class CSerFlagScope
{
private:
	const CSerializationContext *m_context;

public:
	CSerFlagScope(const CSerializationContext &a_context)
		: m_context(&a_context)
	{
		m_context->ToggleFlags(Flags...);
	}
	~CSerFlagScope()
	{
		m_context->ToggleFlags(Flags...);
	}
};

template<SerializationFlags ...Flags>
CSerFlagScope<Flags...> SetSerFlags(const CSerializationContext &a_context)
{
	return CSerFlagScope<Flags...>(a_context);
}

} }

#endif /* SERIALIZATION_CONTEXT_H_ */
