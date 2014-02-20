/*
 * File description: a_serializer.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include "serialization/format/a_serializer.h"

namespace axon { namespace serialization {

ASerializer::~ASerializer()
{
}

size_t ASerializer::CalcSize(const AData &a_data) const
{
	// Fall back to an inefficient implementation that returns
	// the length of the actual serialized string. Some formats
	// don't have a great way to calculate the size of the output
	return SerializeData(a_data).size();
}

size_t ASerializer::SerializeInto(const AData &a_data, char *a_buffer, size_t a_bufferSize) const
{
	auto l_str = SerializeData(a_data);

	if (l_str.size() <= a_bufferSize)
		std::copy(l_str.begin(), l_str.end(), a_buffer);

	return l_str.size();
}

} }


