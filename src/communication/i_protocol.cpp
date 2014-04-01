/*
 * File description: i_protocol.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include "communication/messaging/axon_protocol.h"

namespace axon { namespace communication {

IProtocol::Ptr GetDefaultProtocol()
{
	return IProtocol::Ptr(new CAxonProtocol);
}

}
}


