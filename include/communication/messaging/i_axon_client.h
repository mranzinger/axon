/*
 * File description: i_axon_client.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef I_AXON_CLIENT_H_
#define I_AXON_CLIENT_H_

#include "i_protocol.h"
#include "../i_data_connection.h"
#include "i_contract_host.h"

namespace axon { namespace communication {

class IAxonClient
	: public virtual IContractHost
{
public:
	typedef std::shared_ptr<IAxonClient> Ptr;

	virtual ~IAxonClient() { }

	virtual void Connect(const std::string &a_connectionString) = 0;
	virtual void Connect(IDataConnection::Ptr a_connection) = 0;

	virtual void SetProtocol(IProtocol::Ptr a_protocol) = 0;

	virtual CMessage::Ptr Send(const CMessage &a_message) = 0;
	virtual CMessage::Ptr Send(const CMessage &a_message, uint32_t a_timeout) = 0;
	virtual void SendNonBlocking(const CMessage &a_message) = 0;
};

} }



#endif /* I_AXON_CLIENT_H_ */