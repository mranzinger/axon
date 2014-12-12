/*
 * axon_proxy_server.h
 *
 *  Created on: Dec 12, 2014
 *      Author: mranzinger
 */

#pragma once

#include <atomic>

#include "axon_server.h"
#include "axon_client.h"

namespace axon { namespace communication {

struct CProxyConnection
{
    typedef std::shared_ptr<CProxyConnection> Ptr;

    CAxonClient::Ptr Client;

    std::atomic<int> PendingCount;

    CProxyConnection() = default;
    CProxyConnection(CAxonClient::Ptr a_client)
        : Client(std::move(a_client))
    {
    }
};

struct CProxyPair
{
    std::string      RequestId;
    CAxonClient::Ptr Inbound;
    CProxyConnection::Ptr Outbound;
};

class AXON_COMMUNICATE_API CAxonProxyServer
    : public CAxonServer
{
    class InboundClient;
    class OutboundClient;

    class private_ctor { };

private:
    mutable std::mutex m_clientLock;
    std::vector<CProxyConnection::Ptr> m_openClients;
    std::vector<CProxyConnection::Ptr> m_closedClients;

    mutable std::mutex m_mapLock;
    std::unordered_map<std::string, CProxyPair> m_proxyMap;

public:
    typedef std::shared_ptr<CAxonProxyServer> Ptr;
    typedef std::weak_ptr<CAxonProxyServer> WeakPtr;

    CAxonProxyServer(private_ctor);
    CAxonProxyServer(IProtocolFactory::Ptr a_protoFactory, private_ctor);
    CAxonProxyServer(const std::string &a_hostString, private_ctor);
    CAxonProxyServer(IDataServer::Ptr a_server, private_ctor);
    CAxonProxyServer(const std::string &a_hostString, IProtocolFactory::Ptr a_protoFactory, private_ctor);
    CAxonProxyServer(IDataServer::Ptr a_server, IProtocolFactory::Ptr a_protoFactory, private_ctor);

    static Ptr Create();
    static Ptr Create(IProtocolFactory::Ptr a_protoFactory);
    static Ptr Create(const std::string &a_hostString);
    static Ptr Create(IDataServer::Ptr a_server);
    static Ptr Create(const std::string &a_hostString, IProtocolFactory::Ptr a_protoFactory);
    static Ptr Create(IDataServer::Ptr a_server, IProtocolFactory::Ptr a_protoFactory);

    void AddProxy(IDataConnection::Ptr a_connection);
    void RemoveProxy(const std::string &a_connectionString);

protected:
    virtual CAxonClient::Ptr CreateClient(IDataConnection::Ptr a_client) override;

private:
    void HandleInboundMessage(InboundClient *a_client, const CMessage::Ptr &a_message);
    void HandleOutboundMessage(OutboundClient *a_client, const CMessage::Ptr &a_message);

    CProxyConnection::Ptr SelectOutboundClient();

    void RemoveProxy(std::vector<CProxyConnection::Ptr> &a_conns,
                     const std::string &a_connectionString);
};

} }


