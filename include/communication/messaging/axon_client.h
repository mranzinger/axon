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
#include "i_axon_client.h"

namespace axon { namespace communication {

struct CMessageSocket;

class CAxonClient
	: public AContractHost,
	  public virtual IAxonClient
{
private:
	IDataConnection::Ptr m_connection;
	IProtocol::Ptr m_protocol;

	std::condition_variable m_newMessageEvent;
	std::mutex m_pendingLock;
	std::vector<CMessageSocket*> m_pendingList;

public:
	CAxonClient();
	CAxonClient(const std::string &a_connectionString);
	CAxonClient(IDataConnection::Ptr a_connection);
	CAxonClient(IDataConnection::Ptr a_connection, IProtocol::Ptr a_protocol);

	virtual void Connect(const std::string &a_connectionString) override;
	virtual void Connect(IDataConnection::Ptr a_connection) override;

	void SetDefaultProtocol();
	virtual void SetProtocol(IProtocol::Ptr a_protocol) override;

	virtual CMessage::Ptr Send(const CMessage &a_message) override;
	virtual CMessage::Ptr Send(const CMessage &a_message, uint32_t a_timeout) override;
	virtual void SendNonBlocking(const CMessage &a_message) override;

protected:
	virtual bool TryHandleWithServer(const CMessage &a_msg, CMessage::Ptr &a_out) const;

private:


	void p_OnDataReceived(char *a_buffer, int a_bufferSize);
	void p_OnMessageReceived(const CMessage::Ptr &a_message);
};

} }



#endif /* AXON_CLIENT_H_ */
