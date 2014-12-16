/*
 * i_contract_host.cpp
 *
 *  Created on: Dec 15, 2014
 *      Author: mranzinger
 */

#include "communication/messaging/i_contract_host.h"

namespace axon { namespace communication {

const CContract<std::set<std::string> ()> QUERY_CONTRACTS_CONTRACT
        ("IContractHost.QueryContracts");

} }
