/*
 * File description: serialize_ptr.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef SERIALIZE_PTR_H_
#define SERIALIZE_PTR_H_

#include <memory>

namespace axon { namespace serialization {

template<typename T, bool IsPoly>
struct CPolySerializer
{
	static AData::Ptr Get(const T *a_val, const CSerializationContext &a_context)
	{
		if (a_val)
			return CSerializer<T>::Get(*a_val, a_context);
		else
			return CNullData::Create(a_context);
	}
};

template<typename T>
struct CPolySerializer<T, true>
{
	static AData::Ptr Get(const T *a_val, const CSerializationContext &a_context)
	{
		if (a_val)
		{
			CStructData::Ptr l_ret(new CStructData(a_context));
			CStructWriter l_writer(l_ret.get());

			CPolymorphicBinder<T>::Write(l_writer, a_val);

			return std::move(l_ret);
		}
		else
			return CNullData::Create(a_context);
	}
};

// Create a partial specialization that catches pointers
template<typename T>
struct CSerializer<T*>
	: CPolySerializer<T, CPolymorphicBinder<T>::specialized>
{
};

template<typename T>
struct CSerializer<std::unique_ptr<T>>
{
	static AData::Ptr Get(const std::unique_ptr<T> &a_val, const CSerializationContext &a_context)
	{
		if (a_val)
			return CSerializer<T*>::Get(a_val.get(), a_context);
		else
			return CNullData::Create(a_context);
	}
};

template<typename T>
struct CSerializer<std::shared_ptr<T>>
{
	static AData::Ptr Get(const std::shared_ptr<T> &a_val, const CSerializationContext &a_context)
	{
		if (a_val)
			return CSerializer<T*>::Get(a_val.get(), a_context);
		else
			return CNullData::Create(a_context);
	}
};

template<typename T>
struct CSerializer<std::weak_ptr<T>>
{
	static AData::Ptr Get(const std::weak_ptr<T> &a_val, const CSerializationContext &a_context)
	{
		auto sp = a_val.lock();
		if (sp)
			return Serialize(sp);
		else
			return CNullData::Create(a_context);
	}
};

} }



#endif /* SERIALIZE_PTR_H_ */
