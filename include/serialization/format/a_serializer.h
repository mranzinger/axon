/*
 * File description: i_serializer.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef A_SERIALIZER_H_
#define A_SERIALIZER_H_

#include <memory>
#include <string>

#include "serialization/base/serialize.h"
#include "serialization/base/deserialize.h"

namespace axon { namespace serialization {

class AXON_SERIALIZE_API ASerializer
{
public:
	typedef std::shared_ptr<ASerializer> Ptr;

	virtual ~ASerializer();

	virtual std::string FormatName() const = 0;

	template<typename T>
	std::string Serialize(const T &a_val) const
	{
		auto l_data = axon::serialization::Serialize(a_val);

		return SerializeData(*l_data);
	}

	template<typename T>
	void SerializeToFile(const std::string &a_fileName, const T &a_val) const
	{
		auto l_data = axon::serialization::Serialize(a_val);

		SerializeDataToFile(a_fileName, *l_data);
	}

	template<typename T>
	T Deserialize(const std::string &a_buf) const
	{
		T l_ret;
		Deserialize(a_buf, l_ret);
		return l_ret;
	}
	template<typename T>
	void Deserialize(const std::string &a_buf, T &a_val) const
	{
		auto l_data = DeserializeData(a_buf.data(), a_buf.data() + a_buf.size());

		axon::serialization::Deserialize(*l_data, a_val);
	}

	template<typename T>
	void DeserializeFromFile(const std::string &a_fileName, T &a_val) const
	{
		auto l_data = DeserializeDataFromFile(a_fileName);

		axon::serialization::Deserialize(*l_data, a_val);
	}

	/*
	 * Calculates the amount of space required in bytes to serialize the
	 * specified data.
	 */
	virtual size_t CalcSize(const AData &a_data) const;

	/*
	 * Serializes the specified data, writing it into the supplied buffer.
	 * If the required space to completely serialize the data is larger than the
	 * size of the supplied buffer, then the data inside buffer is undefined.
	 * The return value is the size of the stored data, or the required size of
	 * a_buffer if 'return > a_bufferSize'
	 */
	virtual size_t SerializeInto(const AData &a_data, char *a_buffer, size_t a_bufferSize) const;

	virtual AData::Ptr Deserialize(const std::string &a_str) const;

	virtual AData::Ptr DeserializeData(const char *a_buf, const char *a_endBuf) const = 0;

	/*
	 * Abstract function that serializes the data into the format controlled by
	 * derived classes.
	 */
	virtual std::string SerializeData(const AData &a_data) const = 0;

	virtual void SerializeDataToFile(const std::string &a_fileName, const AData &a_data) const;

	virtual AData::Ptr DeserializeDataFromFile(const std::string &a_fileName) const;
};

} }

#endif /* A_SERIALIZER_H_ */
