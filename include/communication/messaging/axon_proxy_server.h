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
    typedef std::unique_ptr<CProxyConnection> Ptr;

    CAxonClient::Ptr Client;
    std::string      HostName;
    unsigned int     HostPort;

    std::atomic<int> PendingCount;
};

struct CProxyPair
{
    std::string      RequestId;
    CAxonClient::Ptr Inbound;
    CAxonClient::Ptr Outbound;
};

class AXON_COMMUNICATE_API CAxonProxyServer
    : public CAxonServer
{
    class InboundClient;
    class OutboundClient;

private:
    mutable std::mutex m_clientLock;
    std::vector<CProxyConnection::Ptr> m_openClients;
    std::vector<CProxyConnection::Ptr> m_closedClients;

    mutable std::mutex m_mapLock;
    std::unordered_map<std::string, CProxyPair> m_proxyMap;

public:
    typedef std::shared_ptr<CAxonProxyServer> Ptr;
    typedef std::weak_ptr<CAxonProxyServer> WeakPtr;

    static Ptr Create();
    static Ptr Create(IProtocolFactory::Ptr a_protoFactory);
    static Ptr Create(const std::string &a_hostString);
    static Ptr Create(IDataServer::Ptr a_server);
    static Ptr Create(const std::string &a_hostString, IProtocolFactory::Ptr a_protoFactory);
    static Ptr Create(IDataServer::Ptr a_server, IProtocolFactory::Ptr a_protoFactory);

protected:
    virtual CAxonClient::Ptr CreateClient(IDataConnection::Ptr a_client) override;

private:
    CAxonProxyServer();
    CAxonProxyServer(IProtocolFactory::Ptr a_protoFactory);
    CAxonProxyServer(const std::string &a_hostString);
    CAxonProxyServer(IDataServer::Ptr a_server);
    CAxonProxyServer(const std::string &a_hostString, IProtocolFactory::Ptr a_protoFactory);
    CAxonProxyServer(IDataServer::Ptr a_server, IProtocolFactory::Ptr a_protoFactory);

    void HandleInboundMessage(InboundClient *a_client, const CMessage::Ptr &a_message);
    void HandleOutboundMessage(OutboundClient *a_client, const CMessage::Ptr &a_message);

    CProxyConnection *SelectOutboundClient();
};

} }


