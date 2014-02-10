/*
 * i_data_server.cpp
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#include "i_data_server.h"

#include <unordered_map>
#include <string>

namespace axon { namespace communication {

std::unordered_map<std::string, IDataServer::DataServerFactory> s_factoryMap;

IDataServer::Ptr IDataServer::Create( const std::string &hostString )
{
	size_t idx = hostString.find("://");

	std::string prefix = hostString.substr(0, idx);

	auto iter = s_factoryMap.find(prefix);

	if (iter == s_factoryMap.end())
		return IDataServer::Ptr();

	std::string rest;

	if (idx != std::string::npos)
		rest = hostString.substr(idx + 3);

	return iter->second(rest);
}

void IDataServer::RegisterFactory( const std::string &protocolId, const DataServerFactory &f )
{
	s_factoryMap.emplace(protocolId, f);
}

} }


