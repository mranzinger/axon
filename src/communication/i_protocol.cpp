/*
 * File description: i_protocol.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include "communication/messaging/axon_protocol.h"

namespace axon { namespace communication {

IProtocol::Ptr GetDefaultProtocol()
{
	return IProtocol::Ptr(new CAxonProtocol);
}

}
}


