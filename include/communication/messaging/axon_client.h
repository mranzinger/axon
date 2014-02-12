/*
 * File description: axon_client.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef AXON_CLIENT_H_
#define AXON_CLIENT_H_

#include <mutex>

#include "a_contract_host.h"
#include "i_protocol.h"
#include "../i_data_connection.h"

namespace axon { namespace communication {

class CAxonServer;

class CAxonClient
	: public AContractHost
{
private:
	IDataConnection::Ptr m_connection;
	IProtocol::Ptr m_protocol;

	std::condition_variable m_newMessageEvent;
	std::mutex m_newMessageLock;
	std::unordered_map<std::string, CMessage::Ptr> m_newMessages;

public:
	CAxonClient();
	CAxonClient(const std::string &a_connectionString);
	CAxonClient(IDataConnection::Ptr a_connection);

	void Connect(const std::string &a_connectionString);
	void Connect(IDataConnection::Ptr a_connection);

	void SetDefaultProtocol();
	void SetProtocol(IProtocol::Ptr a_protocol);

	CMessage::Ptr Send(const CMessage &a_message, uint32_t a_timeout);
};

} }



#endif /* AXON_CLIENT_H_ */
