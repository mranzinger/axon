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
    return make_shared<CAxonProxyServer>(private_ctor());
}

CAxonProxyServer::Ptr CAxonProxyServer::Create(
        IProtocolFactory::Ptr a_protoFactory)
{
    return make_shared<CAxonProxyServer>(move(a_protoFactory), private_ctor());
}

CAxonProxyServer::Ptr CAxonProxyServer::Create(
        const std::string& a_hostString)
{
    return make_shared<CAxonProxyServer>(a_hostString, private_ctor());
}

CAxonProxyServer::Ptr CAxonProxyServer::Create(IDataServer::Ptr a_server)
{
    return make_shared<CAxonProxyServer>(move(a_server), private_ctor());
}

CAxonProxyServer::Ptr CAxonProxyServer::Create(
        const std::string& a_hostString, IProtocolFactory::Ptr a_protoFactory)
{
    return make_shared<CAxonProxyServer>(a_hostString, move(a_protoFactory), private_ctor());
}

CAxonProxyServer::Ptr CAxonProxyServer::Create(IDataServer::Ptr a_server,
        IProtocolFactory::Ptr a_protoFactory)
{
    return make_shared<CAxonProxyServer>(move(a_server), move(a_protoFactory), private_ctor());
}

CAxonProxyServer::CAxonProxyServer(private_ctor)
{
}

CAxonProxyServer::CAxonProxyServer(
        IProtocolFactory::Ptr a_protoFactory, private_ctor)
    : CAxonServer(move(a_protoFactory))
{
}

CAxonProxyServer::CAxonProxyServer(
        const std::string& a_hostString, private_ctor)
    : CAxonServer(a_hostString)
{
}

CAxonProxyServer::CAxonProxyServer(
        IDataServer::Ptr a_server, private_ctor)
    : CAxonServer(move(a_server))
{
}

CAxonProxyServer::CAxonProxyServer(
        const std::string& a_hostString, IProtocolFactory::Ptr a_protoFactory, private_ctor)
    : CAxonServer(a_hostString, move(a_protoFactory))
{
}



CAxonProxyServer::CAxonProxyServer(
        IDataServer::Ptr a_server, IProtocolFactory::Ptr a_protoFactory, private_ctor)
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
        {
            l_localResponse->SetOneWay(true);
            a_client->SendNonBlocking(l_localResponse);
        }
        return;
    }

    // Otherwise, forward the message to an outbound client
    CProxyConnection::Ptr l_outbound = SelectOutboundClient();

    ++l_outbound->PendingCount;

    if (not a_message->IsOneWay())
    {
        CProxyPair l_p
        {
            a_message->Id(),
            a_client->shared_from_this(),
            l_outbound
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

    --l_p.Outbound->PendingCount;

    // Forward the response
    try
    {
        l_p.Inbound->SendNonBlocking(a_message);
    }
    catch (...)
    {
        // TODO: Decide what to do here
    }
}

CProxyConnection::Ptr CAxonProxyServer::SelectOutboundClient()
{
    lock_guard<mutex> l_lock(m_clientLock);

    if (m_openClients.empty())
        return CProxyConnection::Ptr();

    CProxyConnection::Ptr l_best;
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
            l_best = l_c;
            l_lowestCt = l_pendingCt;
        }
    }

    return l_best;
}

void CAxonProxyServer::AddProxy(IDataConnection::Ptr a_connection)
{
    auto l_client = make_shared<OutboundClient>(
            dynamic_pointer_cast<CAxonProxyServer>(shared_from_this()),
            move(a_connection),
            CreateProtocol()
    );

    lock_guard<mutex> l_lock(m_clientLock);

    if (l_client->IsOpen())
        m_openClients.emplace_back(new CProxyConnection(move(l_client)));
    else
        m_closedClients.emplace_back(new CProxyConnection(move(l_client)));
}

} }
