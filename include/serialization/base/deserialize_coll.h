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

template<typename ValueType, typename CollType>
void ReadCollection(const AData &a_data, CollType &a_coll)
{
	if (a_data.Type() != DataType::Array)
		throw std::runtime_error("Incompatible data type.");

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
