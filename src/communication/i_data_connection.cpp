/*
 * i_data_connection.cpp
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#include "i_data_connection.h"

#include <string>
#include <unordered_map>

namespace axon { namespace communication {

std::unordered_map<std::string, IDataConnection::DataConnectionFactory> s_factoryMap;

IDataConnection::Ptr IDataConnection::Create( const std::string &connectionString )
{
	size_t idx = connectionString.find("://");

	std::string prefix = connectionString.substr(0, idx);

	auto iter = s_factoryMap.find(prefix);

	if (iter == s_factoryMap.end())
		return IDataConnection::Ptr();

	std::string rest;

	if (idx != std::string::npos)
		rest = connectionString.substr(idx + 3);

	return iter->second(rest);
}

void IDataConnection::RegisterFactory( const std::string &prefix, const DataConnectionFactory &f )
{
	s_factoryMap.emplace(prefix, f);
}

} }


