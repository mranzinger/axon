/*
 * File description: serialization_context_impl.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef SERIALIZATION_CONTEXT_IMPL_H_
#define SERIALIZATION_CONTEXT_IMPL_H_

namespace axon { namespace serialization {

class CSerializationContext;

namespace detail {

class CSerializationContextImpl
{
friend class ::axon::serialization::CSerializationContext;

	// Everything is private so that only CSerializationContext can
	// actually manipulate this object
private:
	mutable int m_refCt;

	CSerializationContextImpl() : m_refCt(1) { }

	void AddRef() { ++m_refCt; }
	void RemoveRef()
	{
		--m_refCt;

		if (0 == m_refCt)
			delete this;
	}
};

} // detail

} }



#endif /* SERIALIZATION_CONTEXT_IMPL_H_ */
