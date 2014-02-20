/*
 * File description: deserialize_ptr.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef DESERIALIZE_PTR_H_
#define DESERIALIZE_PTR_H_

#include <memory>

#include "util/and.h"

namespace axon { namespace serialization {

template<typename T, bool IsInstantiable>
struct CInstDeserializer
{
	static void Deserialize(const AData &a_data, T *&a_val)
	{
		if (a_data.Type() == DataType::Null)
		{
			a_val = nullptr;
		}
		else if (nullptr == a_val)
		{
			throw std::runtime_error("Cannot deserialize into a null pointer of a class that is either abstract or doesn't have a default constructor.");
		}
		else
		{
			axon::serialization::Deserialize(a_data, *a_val);
		}
	}
};

template<typename T>
struct CInstDeserializer<T, true>
{
	static void Deserialize(const AData &a_data, T *&a_val)
	{
		try
		{
			if (a_data.Type() == DataType::Null)
			{
				a_val = nullptr;
				return;
			}

			if (!a_val)
				a_val = new T();

			axon::serialization::Deserialize(a_data, *a_val);
		}
		catch (...)
		{
			delete a_val;
			throw;
		}
	}
};

template<typename T, bool IsPoly>
struct CPolyDeserializer
	:  CInstDeserializer<T,
	  	  util::exp_and<
	  	  	  !std::is_abstract<T>::value,
	  	  	  std::is_default_constructible<T>::value>::Value>
{

};

template<typename T>
struct CPolyDeserializer<T, true>
{
	static void Deserialize(const AData &a_data, T *&a_val)
	{
		if (a_data.Type() != DataType::Struct)
			throw std::runtime_error("There is a mismatch between the serialization and deserialization mechanisms."
					" Ensure that the same method is used both ways.");

		auto l_struct = static_cast<const CStructData *>(&a_data);

		CStructReader l_reader(l_struct);

		CPolymorphicBinder<T>::Read(l_reader, a_val);
	}
};

template<typename T>
struct CDeserializer<T*>
	: CPolyDeserializer<T, CPolymorphicBinder<T>::specialized>
{

};

template<typename T>
struct CDeserializer<std::unique_ptr<T>>
{
	static void Deserialize(const AData &a_data, std::unique_ptr<T> &a_val)
	{
		T *l_pVal = a_val.get();

		try
		{
			axon::serialization::Deserialize(a_data, l_pVal);

			if (l_pVal != a_val.get())
				a_val.reset(l_pVal);
		}
		catch (...)
		{
			if (l_pVal != a_val.get())
				delete l_pVal;
			throw;
		}
	}
};

template<typename T>
struct CDeserializer<std::shared_ptr<T>>
{
	static void Deserialize(const AData &a_data, std::shared_ptr<T> &a_val)
	{
		T *l_pVal = a_val.get();

		try
		{
			axon::serialization::Deserialize(a_data, l_pVal);

			if (l_pVal != a_val.get())
				a_val.reset(l_pVal);
		}
		catch (...)
		{
			if (l_pVal != a_val.get())
				delete l_pVal;
			throw;
		}
	}
};

} }



#endif /* DESERIALIZE_PTR_H_ */
