/*
 * File description: axon_server.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef AXON_SERVER_H_
#define AXON_SERVER_H_

#include <mutex>
#include <unordered_map>

#include "a_contract_host.h"
#include "i_protocol_factory.h"
#include "../i_data_server.h"

namespace axon { namespace communication {

class CAxonServerConnection;
typedef std::shared_ptr<CAxonServerConnection> CAxonServerConnectionPtr;

class AXON_COMMUNICATE_API CAxonServer
	: public AContractHost,
	  public std::enable_shared_from_this<CAxonServer>
{
    friend class CAxonServerConnection;

private:
	mutable std::mutex m_clientLock;
	std::unordered_map<IDataConnection*, CAxonServerConnectionPtr> m_clients;

	IDataServer::Ptr m_server;
	IProtocolFactory::Ptr m_proto;
	IProtocol::Ptr m_broadProto;

public:
	typedef std::shared_ptr<CAxonServer> Ptr;
	typedef std::weak_ptr<CAxonServer> WeakPtr;

	static Ptr Create();
	static Ptr Create(IProtocolFactory::Ptr a_protoFactory);
	static Ptr Create(const std::string &a_hostString);
	static Ptr Create(IDataServer::Ptr a_server);
	static Ptr Create(const std::string &a_hostString, IProtocolFactory::Ptr a_protoFactory);
	static Ptr Create(IDataServer::Ptr a_server, IProtocolFactory::Ptr a_protoFactory);

	void Start(const std::string &a_hostString);
	void Start(IDataServer::Ptr a_server);
	void Stop();

	size_t NumClients() const;

	void Broadcast(const CMessage &a_message);

private:
	CAxonServer();
	CAxonServer(IProtocolFactory::Ptr a_protoFactory);
	CAxonServer(const std::string &a_hostString);
	CAxonServer(IDataServer::Ptr a_server);
	CAxonServer(const std::string &a_hostString, IProtocolFactory::Ptr a_protoFactory);
	CAxonServer(IDataServer::Ptr a_server, IProtocolFactory::Ptr a_protoFactory);

	void p_OnClientConnected(IDataConnection::Ptr a_client);
	void p_OnClientDisconnected(IDataConnection::Ptr a_client);
};

} }



#endif /* AXON_SERVER_H_ */
