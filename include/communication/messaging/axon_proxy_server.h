/*
 * axon_proxy_server.h
 *
 *  Created on: Dec 12, 2014
 *      Author: mranzinger
 */

#pragma once

#include <atomic>
#include <random>
#include <set>

#include "axon_server.h"
#include "axon_client.h"

#include "util/timer.h"

namespace axon { namespace communication {

typedef std::shared_ptr<std::atomic<int>> TSharedCt;

struct CProxyConnection
{
    typedef std::shared_ptr<CProxyConnection> Ptr;

    CAxonClient::Ptr Client;
    std::set<std::string> Contracts;

    TSharedCt SharedPendingCount;

    CProxyConnection()
    {
        p_Init();
    }
    CProxyConnection(CAxonClient::Ptr a_client)
        : Client(std::move(a_client))
    {
        p_Init();
    }
    CProxyConnection(CAxonClient::Ptr a_client, TSharedCt a_counter)
        : Client(std::move(a_client)), SharedPendingCount(std::move(a_counter))
    {
        p_Init();
    }

private:
    void p_Init()
    {
        if (not SharedPendingCount)
        {
            SharedPendingCount = std::make_shared<std::atomic<int>>(0);
        }
    }
};

typedef std::vector<CProxyConnection::Ptr> CProxyConnectionList;
typedef std::unordered_map<std::string, CProxyConnectionList> CContractProxyMap;

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
    void AddProxies(const std::vector<IDataConnection::Ptr> &a_conns);
    void RemoveProxy(const std::string &a_connectionString);

    virtual std::set<std::string> QueryContracts() const override;

protected:
    virtual CAxonClient::Ptr CreateClient(IDataConnection::Ptr a_client) override;

private:
    bool HandleInboundMessage(InboundClient *a_client, const CMessage::Ptr &a_message);
    bool HandleOutboundMessage(OutboundClient *a_client, const CMessage::Ptr &a_message);

    CProxyConnection::Ptr SelectOutboundClient(const CMessage &a_message);

    void usRemoveProxy(CContractProxyMap &a_conns,
                       const std::string &a_connectionString);
    void usRemoveProxy(CProxyConnectionList &a_conns,
                       const std::string &a_connectionString);

    void usAddToOpenClients(CProxyConnection::Ptr a_conn);
    void usAddToDisconnected(CProxyConnection::Ptr a_conn);

    void p_InitTimer();
    void p_OnTimerTicked();

private:
    mutable std::mutex m_clientLock;
    CContractProxyMap m_openClients;
    CProxyConnectionList m_closedClients;
    std::mt19937 m_selectRand;

    mutable std::mutex m_mapLock;
    std::unordered_map<std::string, CProxyPair> m_proxyMap;

    util::Timer m_timer;
};

} }


