/*
 * File description: axon_server.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include "communication/messaging/axon_server.h"
#include "communication/messaging/axon_client.h"

using namespace std;

namespace axon { namespace communication {

class CAxonServerConnection
	: public CAxonClient
{
private:
	CAxonServer::WeakPtr m_parent;

public:
	CAxonServerConnection(const CAxonServer::Ptr &a_parent, IDataConnection::Ptr a_connection,
			IProtocol::Ptr a_protocol)
		: CAxonClient(move(a_connection), move(a_protocol), protected_ctor()), m_parent(a_parent)
	{

	}

	virtual void Connect(const std::string &a_connectionString) override
	{
		throw runtime_error("Cannot change the connection of a server managed client.");
	}
	virtual void Connect(IDataConnection::Ptr a_connection) override
	{
		throw runtime_error("Cannot change the connection of a server managed client.");
	}

protected:
	virtual bool TryHandleWithServer(const CMessage &a_msg, CMessage::Ptr &a_out) const override
	{
		auto l_parent = m_parent.lock();

		if (!l_parent)
		{
			// Somehow this connection got orphaned. Either way, just ignore
			// and say it wasn't handled.
			// TODO: Log this
			return false;
		}

		// Let the server try to handle it
		return l_parent->TryHandle(a_msg, a_out);
	}
};



void CAxonServer::Start(const std::string& a_hostString)
{
	Start(IDataServer::Create(a_hostString));
}

void CAxonServer::Start(IDataServer::Ptr a_server)
{
	if (!a_server)
		throw runtime_error("Cannot start with a null server.");

	m_server = move(a_server);

	m_server->SetConnectedHandler(bind(&CAxonServer::p_OnClientConnected, this, placeholders::_1));
	m_server->SetDisconnectedHandler(bind(&CAxonServer::p_OnClientDisconnected, this, placeholders::_1));
}

void CAxonServer::Stop()
{
	if (m_server)
		m_server->Shutdown();
}

size_t CAxonServer::NumClients() const
{
	lock_guard<mutex> l_lock(m_clientLock);
	return m_clients.size();
}

void CAxonServer::Broadcast(const CMessage& a_message)
{
	if (!m_server)
		throw runtime_error("Cannot broadcast using a dead server.");

	if (!m_broadProto)
		m_broadProto = m_proto->Create();

	util::CBuffer l_buff = m_broadProto->SerializeMessage(a_message).ToShared();

	m_server->Broadcast(l_buff);
}

void CAxonServer::p_OnClientConnected(IDataConnection::Ptr a_client)
{
	auto lp = a_client.get();
	auto l_conn = CreateClient(move(a_client));

	lock_guard<mutex> l_lock(m_clientLock);
	m_clients.emplace(lp, move(l_conn));
}

CAxonClient::Ptr CAxonServer::CreateClient(IDataConnection::Ptr a_client)
{
    return make_shared<CAxonServerConnection>(shared_from_this(),
                                              move(a_client),
                                              CreateProtocol());
}

IProtocol::Ptr CAxonServer::CreateProtocol()
{
    return m_proto->Create();
}

void CAxonServer::p_OnClientDisconnected(IDataConnection::Ptr a_client)
{
	lock_guard<mutex> l_lock(m_clientLock);

	m_clients.erase(a_client.get());
}

CAxonServer::CAxonServer()
	: m_proto(GetDefaultProtocolFactory())
{
}

CAxonServer::CAxonServer(IProtocolFactory::Ptr a_protoFactory)
	: m_proto(move(a_protoFactory))
{
}

CAxonServer::CAxonServer(const std::string& a_hostString)
	: m_proto(GetDefaultProtocolFactory())
{
	Start(a_hostString);
}

CAxonServer::CAxonServer(IDataServer::Ptr a_server)
	: m_proto(GetDefaultProtocolFactory())
{
	Start(move(a_server));
}

CAxonServer::CAxonServer(const std::string& a_hostString, IProtocolFactory::Ptr a_protoFactory)
	: m_proto(move(a_protoFactory))
{
	Start(a_hostString);
}

CAxonServer::CAxonServer(IDataServer::Ptr a_server, IProtocolFactory::Ptr a_protoFactory)
	: m_proto(move(a_protoFactory))
{
	Start(move(a_server));
}

CAxonServer::Ptr CAxonServer::Create()
{
	return CAxonServer::Ptr(new CAxonServer());
}

CAxonServer::Ptr CAxonServer::Create(IProtocolFactory::Ptr a_protoFactory)
{
	return CAxonServer::Ptr(new CAxonServer(move(a_protoFactory)));
}

CAxonServer::Ptr CAxonServer::Create(const std::string& a_hostString)
{
	return CAxonServer::Ptr(new CAxonServer(a_hostString));
}

CAxonServer::Ptr CAxonServer::Create(IDataServer::Ptr a_server)
{
	return CAxonServer::Ptr(new CAxonServer(move(a_server)));
}

CAxonServer::Ptr CAxonServer::Create(const std::string& a_hostString, IProtocolFactory::Ptr a_protoFactory)
{
	return CAxonServer::Ptr(new CAxonServer(a_hostString, move(a_protoFactory)));
}

CAxonServer::Ptr CAxonServer::Create(IDataServer::Ptr a_server, IProtocolFactory::Ptr a_protoFactory)
{
	return CAxonServer::Ptr(new CAxonServer(move(a_server), move(a_protoFactory)));
}



}
}


