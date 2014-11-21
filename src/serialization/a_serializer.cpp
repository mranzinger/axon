/*
 * File description: a_serializer.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include "serialization/format/a_serializer.h"

#include <fstream>

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

void ASerializer::SerializeDataToFile(const std::string &a_fileName, const AData &a_data) const
{
	std::string l_ser = SerializeData(a_data);

	std::ofstream fs(a_fileName, std::ios_base::binary);

	fs.write(l_ser.c_str(), l_ser.size());
}

AData::Ptr ASerializer::DeserializeDataFromFile(const std::string &a_fileName) const
{
	std::ifstream fs(a_fileName, std::ios_base::binary);

	fs.seekg(0, fs.end);

	size_t len = fs.tellg();

	fs.seekg(0, fs.beg);

	std::string l_str(len, '\0');

	fs.read(&l_str[0], len);

	return DeserializeData(l_str.data(), l_str.data() + len);
}

AData::Ptr ASerializer::Deserialize(const std::string& a_str) const
{
	return DeserializeData(a_str.c_str(), a_str.c_str() + a_str.size());
}

}
}


