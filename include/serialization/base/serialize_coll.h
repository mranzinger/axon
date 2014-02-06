/*
 * File description: serialize_coll.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef SERIALIZE_COLL_H_
#define SERIALIZE_COLL_H_

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>

namespace axon { namespace serialization {

template<typename Key, typename Value>
void WriteStruct(CStructWriter &a_writer, const std::pair<Key, Value> &a_val)
{
	a_writer("k", a_val.first)("v", a_val.second);
}

template<typename IterType>
void SerializeIter(CArrayData &a_data, IterType iter, IterType end, const CSerializationContext &a_context)
{
	for (; iter != end; ++iter)
		a_data.Add(Serialize(*iter, a_context));
}

template<typename CollType>
void SerializeColl(CArrayData &a_data, const CollType &a_coll, const CSerializationContext &a_context)
{
	SerializeIter(a_data, std::begin(a_coll), std::end(a_coll), a_context);
}

template<typename CollType>
AData::Ptr SerializeCollImpl(const CollType &a_coll, const CSerializationContext &a_context, util::detail::sfinae_base::no)
{
	CArrayData::Ptr l_ret(new CArrayData(a_context));
	SerializeColl(*l_ret, a_coll, a_context);
	return std::move(l_ret);
}

template<typename CollType>
AData::Ptr SerializeCollImpl(const CollType &a_coll,
		const CSerializationContext &a_context, util::detail::sfinae_base::yes)
{
	typedef typename CollType::value_type value_type;
	typedef CPrimArrayData<value_type> data_type;
	typedef typename data_type::Ptr data_ptr;

	data_ptr l_ret(new data_type(a_context));
	l_ret->Import(a_coll);
	return std::move(l_ret);
}

template<typename CollType>
AData::Ptr SerializeColl(const CollType &a_coll, const CSerializationContext &a_context)
{
	return SerializeCollImpl(a_coll, a_context, typename CDataTypeTraits<typename CollType::value_type>::is_primitive_type());
}

template<typename T>
struct CSerializer<std::vector<T>>
{
	static AData::Ptr Get(const std::vector<T> &a_val, const CSerializationContext &a_context)
	{
		return SerializeColl(a_val, a_context);
	}
};

template<typename T>
struct CSerializer<std::set<T>>
{
	static AData::Ptr Get(const std::set<T> &a_val, const CSerializationContext &a_context)
	{
		return SerializeColl(a_val, a_context);
	}
};

template<typename T>
struct CSerializer<std::unordered_set<T>>
{
	static AData::Ptr Get(const std::unordered_set<T> &a_val, const CSerializationContext &a_context)
	{
		return SerializeColl(a_val, a_context);
	}
};

template<typename T>
struct CSerializer<std::list<T>>
{
	static AData::Ptr Get(const std::list<T> &a_val, const CSerializationContext &a_context)
	{
		return SerializeColl(a_val, a_context);
	}
};

template<typename Key, typename Value>
struct CSerializer<std::map<Key, Value>>
{
	static AData::Ptr Get(const std::map<Key, Value> &a_val, const CSerializationContext &a_context)
	{
		return SerializeColl(a_val, a_context);
	}
};

template<typename Key, typename Value>
struct CSerializer<std::unordered_map<Key, Value>>
{
	static AData::Ptr Get(const std::unordered_map<Key, Value> &a_val, const CSerializationContext &a_context)
	{
		return SerializeColl(a_val, a_context);
	}
};

} }



#endif /* SERIALIZE_COLL_H_ */
