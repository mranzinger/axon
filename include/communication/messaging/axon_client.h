/*
 * File description: axon_client.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
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

class AXON_COMMUNICATE_API CAxonClient
	: public AContractHost,
	  public virtual IAxonClient,
	  public std::enable_shared_from_this<CAxonClient>
{
    class WaitHandle;

private:
	IDataConnection::Ptr m_connection;
	IProtocol::Ptr m_protocol;

	std::condition_variable m_newMessageEvent;
	std::mutex m_pendingLock;
	std::vector<CMessageSocket*> m_pendingList;

protected:
	struct protected_ctor { };

public:
	CAxonClient(protected_ctor);
	CAxonClient(const std::string &a_connectionString, protected_ctor);
	CAxonClient(IDataConnection::Ptr a_connection, protected_ctor);
	CAxonClient(const std::string &a_connectionString, IProtocol::Ptr a_protocol, protected_ctor);
	CAxonClient(IDataConnection::Ptr a_connection, IProtocol::Ptr a_protocol, protected_ctor);

	static Ptr Create();
	static Ptr Create(const std::string &a_connectionString);
	static Ptr Create(IDataConnection::Ptr a_connection);
	static Ptr Create(const std::string &a_connectionString, IProtocol::Ptr a_protocol);
	static Ptr Create(IDataConnection::Ptr a_connection, IProtocol::Ptr a_protocol);

	virtual void Connect(const std::string &a_connectionString) override;
	virtual void Connect(IDataConnection::Ptr a_connection) override;
	virtual bool Reconnect() override;
	virtual void Close() override;

	virtual bool IsOpen() const override { return m_connection->IsOpen(); }

	void SetDefaultProtocol();
	virtual void SetProtocol(IProtocol::Ptr a_protocol) override;

    std::string ConnectionString() const override;

	virtual CMessage::Ptr Send(const CMessage::Ptr &a_message) override;
	virtual CMessage::Ptr Send(const CMessage::Ptr &a_message, uint32_t a_timeout) override;
	virtual IMessageWaitHandle::Ptr SendAsync(const CMessage::Ptr &a_message) override;
    virtual IMessageWaitHandle::Ptr SendAsync(const CMessage::Ptr &a_message, uint32_t a_timeout) override;
	virtual void SendNonBlocking(const CMessage::Ptr &a_message) override;

protected:
	virtual bool TryHandleWithServer(const CMessage::Ptr &a_msg, CMessage::Ptr &a_out) const;
	virtual void HandleProtocolError(std::exception &ex);
	virtual void OnMessageReceived(const CMessage::Ptr &a_message);

private:
	void p_Send(const CMessage &a_message);

	void p_OnDataReceived(CDataBuffer a_buffer);
	void p_OnMessageReceived(const CMessage::Ptr &a_message);
};

} }



#endif /* AXON_CLIENT_H_ */
