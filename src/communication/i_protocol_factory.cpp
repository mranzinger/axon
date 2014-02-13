/*
 * File description: i_protocol_factory.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include "communication/messaging/i_protocol_factory.h"
#include "communication/messaging/axon_protocol.h"

namespace axon { namespace communication {

IProtocolFactory::Ptr GetDefaultProtocolFactory()
{
	return GetProtocolFactory<CAxonProtocol>();
}

}
}


