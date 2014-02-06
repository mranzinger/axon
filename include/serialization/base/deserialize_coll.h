/*
 * File description: deserialize_coll.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef DESERIALIZE_COLL_H_
#define DESERIALIZE_COLL_H_

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>

namespace axon { namespace serialization {

template<typename Key, typename Value>
void ReadStruct(const CStructReader &a_reader, std::pair<Key, Value> &a_val)
{
	a_reader("k", a_val.first)("v", a_val.second);
}

template<typename ValueType, typename CollType, typename DataType>
void ReadPrimCollection(const CPrimArrayData<DataType> &a_data, CollType &a_coll, util::detail::sfinae_base::no)
{
	// This will get called if ValueType is not primitive
	throw std::runtime_error("Unable to convert a primitive array to the specified type.");
}

template<typename ValueType, typename CollType, typename DataType>
void ReadPrimCollectionImpl(const CPrimArrayData<DataType> &a_data, CollType &a_coll, std::true_type)
{
	// Handler for implicit conversions
	a_coll = CollType(a_data.begin(), a_data.end());
}

template<typename ValueType, typename CollType, typename DataType>
void ReadPrimCollectionImpl(const CPrimArrayData<DataType> &a_data, CollType &a_coll, std::false_type)
{
	for (const DataType &l_dt : a_data)
	{
		a_coll.insert(a_coll.end(), cast_to<ValueType>(l_dt));
	}
}

template<typename ValueType, typename CollType, typename DataType>
void ReadPrimCollection(const CPrimArrayData<DataType> &a_data, CollType &a_coll, util::detail::sfinae_base::yes)
{
	ReadPrimCollectionImpl<ValueType, CollType, DataType>(a_data, a_coll,
			typename std::is_convertible<DataType, ValueType>::type());
}

template<typename ValueType, typename CollType>
void ReadCollection(const AData &a_data, CollType &a_coll)
{
	if (a_data.Type() == DataType::PrimArray)
	{
#define READ_PRIM(name, type) \
		case DataType::name: \
			ReadPrimCollection<ValueType, CollType, type>( \
					static_cast<const CPrimArrayData<type> &>(a_data), a_coll, \
					typename CDataTypeTraits<ValueType>::is_primitive_type()); \
			break

		switch (static_cast<const APrimArrayDataBase &>(a_data).InnerType())
		{
		READ_PRIM(SByte, signed char);
		READ_PRIM(UByte, unsigned char);
		READ_PRIM(Short, short);
		READ_PRIM(UShort, unsigned short);
		READ_PRIM(Int, int);
		READ_PRIM(UInt, unsigned int);
		READ_PRIM(Long, long long);
		READ_PRIM(ULong, unsigned long);
		READ_PRIM(Float, float);
		READ_PRIM(Double, double);
		READ_PRIM(Bool, bool);
		READ_PRIM(String, std::string);

		default:
			throw std::runtime_error("Unsupported primitive data type.");
		}

#undef READ_PRIM
	}
	else if (a_data.Type() == DataType::Array)
	{
		auto l_arr = static_cast<const CArrayData *>(&a_data);

		// Get an iterator that can be used to add elements to the collection
		auto l_insIter = std::inserter(a_coll, a_coll.end());

		for (const AData::Ptr &l_child : *l_arr)
		{
			ValueType l_cv;
			Deserialize(*l_child, l_cv);

			*l_insIter++ = std::move(l_cv);
		}
	}
	else
		throw std::runtime_error("Incompatible data type.");
}

template<typename CollType>
void ReadCollection(const AData &a_data, CollType &a_coll)
{
	typedef typename CollType::value_type value_type;

	ReadCollection<value_type>(a_data, a_coll);
}

template<typename T>
struct CDeserializer<std::vector<T>>
{
	static void Deserialize(const AData &a_data, std::vector<T> &a_coll)
	{
		ReadCollection(a_data, a_coll);
	}
};

template<typename T>
struct CDeserializer<std::list<T>>
{
	static void Deserialize(const AData &a_data, std::list<T> &a_coll)
	{
		ReadCollection(a_data, a_coll);
	}
};

template<typename T>
struct CDeserializer<std::set<T>>
{
	static void Deserialize(const AData &a_data, std::set<T> &a_coll)
	{
		ReadCollection(a_data, a_coll);
	}
};

template<typename T>
struct CDeserializer<std::unordered_set<T>>
{
	static void Deserialize(const AData &a_data, std::unordered_set<T> &a_coll)
	{
		ReadCollection(a_data, a_coll);
	}
};

template<typename Key, typename Value>
struct CDeserializer<std::map<Key, Value>>
{
	static void Deserialize(const AData &a_data, std::map<Key, Value> &a_coll)
	{
		ReadCollection<std::pair<Key, Value>>(a_data, a_coll);
	}
};

template<typename Key, typename Value>
struct CDeserializer<std::unordered_map<Key, Value>>
{
	static void Deserialize(const AData &a_data, std::unordered_map<Key, Value> &a_coll)
	{
		ReadCollection<std::pair<Key, Value>>(a_data, a_coll);
	}
};

} }



#endif /* DESERIALIZE_COLL_H_ */
