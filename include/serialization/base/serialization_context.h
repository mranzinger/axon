/*
 * File description: serialization_context.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef SERIALIZATION_CONTEXT_H_
#define SERIALIZATION_CONTEXT_H_

#include <stdint.h>

#include "detail/serialization_context_impl.h"
#include "../dll_export.h"

namespace axon { namespace serialization {

enum class SerializationFlags : uint32_t
{
	None = 0,
	Shareable = 0x1,
	Compress = 0x2
};

class AXON_SERIALIZE_API CSerializationContext
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

class AXON_SERIALIZE_API CSerFlagScope
{
private:
	const CSerializationContext *m_context;
	SerializationFlags m_flags;

public:
	template<typename ...Flags>
	CSerFlagScope(const CSerializationContext &a_context, Flags ...a_flags)
		: m_context(&a_context), m_flags(SerializationFlags::None)
	{
		AppendFlags(a_flags...);

		m_context->ToggleFlags(m_flags);
	}
	~CSerFlagScope()
	{
		m_context->ToggleFlags(m_flags);
	}

	void AppendFlags() { }

	template<typename ...Flags>
	void AppendFlags(SerializationFlags a_flag, Flags ...a_rest)
	{
		m_flags = (SerializationFlags)(uint32_t(a_flag) ^ uint32_t(m_flags));

		AppendFlags(a_rest...);
	}
};

template<typename ...Flags>
CSerFlagScope SetSerFlags(const CSerializationContext &a_context, SerializationFlags a_first, Flags ...a_flags)
{
	return CSerFlagScope(a_context, a_first, a_flags...);
}

inline int SetSerFlags(const CSerializationContext &a_context) { return 42; }

} }

#endif /* SERIALIZATION_CONTEXT_H_ */
