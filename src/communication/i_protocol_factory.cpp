/*
 * File description: i_protocol_factory.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
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


