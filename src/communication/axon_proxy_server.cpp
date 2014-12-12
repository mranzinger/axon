/*
 * axon_proxy_server.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: mranzinger
 */

#include "communication/messaging/axon_proxy_server.h"

using namespace std;

namespace axon { namespace communication {

class CAxonProxyServer::InboundClient
    : public CAxonClient
{
private:
    CAxonProxyServer::WeakPtr m_parent;

public:
    InboundClient(const CAxonProxyServer::Ptr &a_parent, IDataConnection::Ptr a_connection,
           IProtocol::Ptr a_protocol)
        : CAxonClient(move(a_connection), move(a_protocol), protected_ctor()),
          m_parent(a_parent)
    {
    }

protected:
    virtual void OnMessageReceived(const CMessage::Ptr &a_message)
    {
        auto l_parent = m_parent.lock();

        if (!l_parent)
            return;

        l_parent->HandleInboundMessage(this, a_message);
    }
};

class CAxonProxyServer::OutboundClient
    : public CAxonClient
{
private:
    CAxonProxyServer::WeakPtr m_parent;

public:
    OutboundClient(const CAxonProxyServer::Ptr &a_parent, IDataConnection::Ptr a_connection,
                   IProtocol::Ptr a_protocol)
        : CAxonClient(move(a_connection), move(a_protocol), protected_ctor()),
          m_parent(a_parent)
    {
    }

protected:
    virtual void OnMessageReceived(const CMessage::Ptr &a_message)
    {
        auto l_parent = m_parent.lock();

        if (!l_parent)
            return;

        l_parent->HandleOutboundMessage(this, a_message);
    }
};

CAxonProxyServer::Ptr CAxonProxyServer::Create()
{
    return make_shared<CAxonProxyServer>();
}

CAxonProxyServer::Ptr CAxonProxyServer::Create(
        IProtocolFactory::Ptr a_protoFactory)
{
    return make_shared<CAxonProxyServer>(move(a_protoFactory));
}

CAxonProxyServer::Ptr CAxonProxyServer::Create(
        const std::string& a_hostString)
{
    return make_shared<CAxonProxyServer>(a_hostString);
}

CAxonProxyServer::Ptr CAxonProxyServer::Create(IDataServer::Ptr a_server)
{
    return make_shared<CAxonProxyServer>(move(a_server));
}

CAxonProxyServer::Ptr CAxonProxyServer::Create(
        const std::string& a_hostString, IProtocolFactory::Ptr a_protoFactory)
{
    return make_shared<CAxonProxyServer>(a_hostString, move(a_protoFactory));
}

CAxonProxyServer::Ptr CAxonProxyServer::Create(IDataServer::Ptr a_server,
        IProtocolFactory::Ptr a_protoFactory)
{
    return make_shared<CAxonProxyServer>(move(a_server), move(a_protoFactory));
}

CAxonProxyServer::CAxonProxyServer()
{
}

CAxonProxyServer::CAxonProxyServer(
        IProtocolFactory::Ptr a_protoFactory)
    : CAxonServer(move(a_protoFactory))
{
}

CAxonProxyServer::CAxonProxyServer(
        const std::string& a_hostString)
    : CAxonServer(a_hostString)
{
}

CAxonProxyServer::CAxonProxyServer(
        IDataServer::Ptr a_server)
    : CAxonServer(move(a_server))
{
}

CAxonProxyServer::CAxonProxyServer(
        const std::string& a_hostString, IProtocolFactory::Ptr a_protoFactory)
    : CAxonServer(a_hostString, move(a_protoFactory))
{
}



CAxonProxyServer::CAxonProxyServer(
        IDataServer::Ptr a_server, IProtocolFactory::Ptr a_protoFactory)
    : CAxonServer(move(a_server), move(a_protoFactory))
{
}

CAxonClient::Ptr CAxonProxyServer::CreateClient(IDataConnection::Ptr a_client)
{
    return make_shared<InboundClient>(dynamic_pointer_cast<CAxonProxyServer>(shared_from_this()),
                                      move(a_client),
                                      CreateProtocol());
}

void CAxonProxyServer::HandleInboundMessage(InboundClient* a_client,
                                             const CMessage::Ptr& a_message)
{
    // Check to see if this can be handled locally
    CMessage::Ptr l_localResponse;
    if (TryHandle(*a_message, l_localResponse))
    {
        if (!a_message->IsOneWay())
            a_client->SendNonBlocking(l_localResponse);
        return;
    }

    // Otherwise, forward the message to an outbound client
    CProxyConnection *l_outbound = SelectOutboundClient();

    ++l_outbound->PendingCount;

    if (not a_message->IsOneWay())
    {
        CProxyPair l_p
        {
            a_message->Id(),
            a_client->shared_from_this(),
            l_outbound->Client
        };

        lock_guard<mutex> l_lock(m_mapLock);
        m_proxyMap.emplace(l_p.RequestId, move(l_p));
    }

    // TODO: Need to catch errors here
    l_outbound->Client->SendNonBlocking(a_message);
}

void CAxonProxyServer::HandleOutboundMessage(OutboundClient* a_client,
                                             const CMessage::Ptr& a_message)
{
    CProxyPair l_p;
    {
        lock_guard<mutex> l_lock(m_mapLock);

        auto l_iter = m_proxyMap.find(a_message->RequestId());

        if (l_iter == m_proxyMap.end())
            return;

        l_p = l_iter->second;
        m_proxyMap.erase(l_iter);
    }


}

CProxyConnection *CAxonProxyServer::SelectOutboundClient()
{
    lock_guard<mutex> l_lock(m_clientLock);

    if (m_openClients.empty())
        return CAxonClient::Ptr();

    CProxyConnection *l_best = nullptr;
    int l_lowestCt  = numeric_limits<int>::max();

    for (int i = m_openClients.size() - 1; i >= 0; --i)
    {
        CProxyConnection::Ptr &l_c = m_openClients[i];

        if (not l_c->Client->IsOpen())
        {
            m_closedClients.push_back(move(l_c));
            m_openClients.erase(begin(m_openClients) + i);
            continue;
        }

        int l_pendingCt = l_c->PendingCount;

        if (not l_best || l_pendingCt < l_lowestCt)
        {
            l_best = l_c.get();
            l_lowestCt = l_pendingCt;
        }
    }

    return l_best;
}

} }
