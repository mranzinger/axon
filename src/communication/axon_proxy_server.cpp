/*
 * axon_proxy_server.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: mranzinger
 */

#include "communication/messaging/axon_proxy_server.h"

#include <iostream>

#include "communication/disconnected_exception.h"

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

    virtual set<string> QueryContracts() const override
    {
        set<string> l_ret = CAxonClient::QueryContracts();

        auto l_parent = m_parent.lock();

        if (not l_parent)
            return move(l_ret);

        set<string> l_server = l_parent->QueryContracts();

        l_ret.insert(begin(l_server), end(l_server));

        return move(l_ret);
    }

protected:
    virtual bool TryHandleWithServer(const CMessage::Ptr &a_message, CMessage::Ptr &a_out) const override
    {
        auto l_parent = m_parent.lock();

        if (!l_parent)
            return false;

        return l_parent->HandleInboundMessage(const_cast<InboundClient*>(this),
                                              a_message);
    }
};

class CAxonProxyServer::OutboundClient
    : public CAxonClient
{
private:
    CAxonProxyServer::WeakPtr m_parent;

public:
    OutboundClient(const CAxonProxyServer::Ptr &a_parent,
                   IDataConnection::Ptr a_connection,
                   IProtocol::Ptr a_protocol)
        : CAxonClient(move(a_connection), move(a_protocol), protected_ctor()),
          m_parent(a_parent)
    {
    }

protected:
    virtual bool TryHandleWithServer(const CMessage::Ptr &a_message, CMessage::Ptr &a_out) const override
    {
        auto l_parent = m_parent.lock();

        if (!l_parent)
            return false;

        return l_parent->HandleOutboundMessage(const_cast<OutboundClient*>(this),
                                               a_message);
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
    p_InitTimer();
}

CAxonProxyServer::CAxonProxyServer(
        IProtocolFactory::Ptr a_protoFactory, private_ctor)
    : CAxonServer(move(a_protoFactory))
{
    p_InitTimer();
}

CAxonProxyServer::CAxonProxyServer(
        const std::string& a_hostString, private_ctor)
    : CAxonServer(a_hostString)
{
    p_InitTimer();
}

CAxonProxyServer::CAxonProxyServer(
        IDataServer::Ptr a_server, private_ctor)
    : CAxonServer(move(a_server))
{
    p_InitTimer();
}

CAxonProxyServer::CAxonProxyServer(
        const std::string& a_hostString, IProtocolFactory::Ptr a_protoFactory, private_ctor)
    : CAxonServer(a_hostString, move(a_protoFactory))
{
    p_InitTimer();
}

CAxonProxyServer::CAxonProxyServer(
        IDataServer::Ptr a_server, IProtocolFactory::Ptr a_protoFactory, private_ctor)
    : CAxonServer(move(a_server), move(a_protoFactory))
{
    p_InitTimer();
}

void CAxonProxyServer::p_InitTimer()
{
    m_timer.SetInterval(chrono::milliseconds(10000), false);
    m_timer.SetCallback(bind(&CAxonProxyServer::p_OnTimerTicked, this));

    m_timer.Start();
}

CAxonClient::Ptr CAxonProxyServer::CreateClient(IDataConnection::Ptr a_client)
{
    return make_shared<InboundClient>(dynamic_pointer_cast<CAxonProxyServer>(shared_from_this()),
                                      move(a_client),
                                      CreateProtocol());
}

bool CAxonProxyServer::HandleInboundMessage(InboundClient* a_client,
                                             const CMessage::Ptr& a_message)
{
    // Check to see if this can be handled locally
    CMessage::Ptr l_localResponse;
    if (TryHandle(*a_message, l_localResponse))
    {
        if (not a_message->IsOneWay())
        {
            l_localResponse->SetOneWay(true);
            a_client->SendNonBlocking(l_localResponse);
        }
        return true;
    }

    // Otherwise, forward the message to an outbound client
    CProxyConnection::Ptr l_outbound = SelectOutboundClient(*a_message);

    if (not l_outbound)
        return false;

    ++(*l_outbound->SharedPendingCount);

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

    return true;
}

bool CAxonProxyServer::HandleOutboundMessage(OutboundClient* a_client,
                                             const CMessage::Ptr& a_message)
{
    // Check to see if this can be handled locally
    CMessage::Ptr l_localResponse;
    if (TryHandle(*a_message, l_localResponse))
    {
        if (not a_message->IsOneWay())
        {
            l_localResponse->SetOneWay(true);
            a_client->SendNonBlocking(l_localResponse);
        }
        return true;
    }

    CProxyPair l_p;
    {
        lock_guard<mutex> l_lock(m_mapLock);

        auto l_iter = m_proxyMap.find(a_message->RequestId());

        if (l_iter == m_proxyMap.end())
            return false;

        l_p = l_iter->second;
        m_proxyMap.erase(l_iter);
    }

    --(*l_p.Outbound->SharedPendingCount);

    // Forward the response
    try
    {
        l_p.Inbound->SendNonBlocking(a_message);
    }
    catch (...)
    {
        // TODO: Decide what to do here
    }
    return true;
}

CProxyConnection::Ptr CAxonProxyServer::SelectOutboundClient(const CMessage &a_message)
{
    lock_guard<mutex> l_lock(m_clientLock);

    auto l_iter = m_openClients.find(a_message.GetAction());

    if (l_iter == m_openClients.end())
        return CProxyConnection::Ptr();

    CProxyConnectionList &l_conns = l_iter->second;

    if (l_conns.empty())
        return CProxyConnection::Ptr();

    CProxyConnectionList l_best;
    int l_lowestCt  = numeric_limits<int>::max();

    for (int i = l_conns.size() - 1; i >= 0; --i)
    {
        CProxyConnection::Ptr &l_c = l_conns[i];

        if (not l_c->Client->IsOpen())
        {
            usAddToDisconnected(move(l_c));
            l_conns.erase(begin(l_conns) + i);
            continue;
        }

        int l_pendingCt = *l_c->SharedPendingCount;

        if (l_best.empty() || l_pendingCt < l_lowestCt)
        {
            l_best.clear();
            l_best.push_back(l_c);
            l_lowestCt = l_pendingCt;
        }
        else if (l_pendingCt == l_lowestCt)
        {
            l_best.push_back(l_c);
        }
    }

    size_t l_selectIdx = m_selectRand() % l_best.size();

    return move(l_best[l_selectIdx]);
}

void CAxonProxyServer::AddProxy(IDataConnection::Ptr a_connection)
{
    AddProxies({ a_connection });
}

void CAxonProxyServer::AddProxies(const vector<IDataConnection::Ptr> &a_conns)
{
    auto l_counter = make_shared<atomic<int>>(0);

    for (const IDataConnection::Ptr &l_conn : a_conns)
    {
        auto l_client = make_shared<OutboundClient>(
                dynamic_pointer_cast<CAxonProxyServer>(shared_from_this()),
                l_conn,
                CreateProtocol()
        );

        auto l_proxyConn = make_shared<CProxyConnection>(move(l_client), l_counter);

        bool l_good = EstablishContracts(*l_proxyConn);

        lock_guard<mutex> l_lock(m_clientLock);

        if (l_good)
            usAddToOpenClients(move(l_proxyConn));
        else
            usAddToDisconnected(move(l_proxyConn));
    }
}

void CAxonProxyServer::RemoveProxy(const string &a_connectionString)
{
    lock_guard<mutex> l_lock(m_clientLock);

    usRemoveProxy(m_openClients, a_connectionString);
    usRemoveProxy(m_closedClients, a_connectionString);
}

void CAxonProxyServer::usRemoveProxy(CContractProxyMap& a_conns,
                                   const std::string& a_connectionString)
{
    for (pair<const string, CProxyConnectionList> &a_c : a_conns)
    {
        usRemoveProxy(a_c.second, a_connectionString);
    }
}

void CAxonProxyServer::usRemoveProxy(std::vector<CProxyConnection::Ptr>& a_conns,
                                   const std::string& a_connectionString)
{
    if (a_conns.empty())
        return;

    for (int i = a_conns.size() - 1; i >= 0; --i)
    {
        CProxyConnection::Ptr &l_conn = a_conns[i];

        if (l_conn->Client->ConnectionString().find(a_connectionString) !=
            string::npos)
        {
            a_conns.erase(begin(a_conns) + i);
        }
    }
}

set<string> CAxonProxyServer::QueryContracts() const
{
    set<string> l_ret = CAxonServer::QueryContracts();

    {
        lock_guard<mutex> l_lock(m_clientLock);

        for (const pair<const string, CProxyConnectionList> &a_c : m_openClients)
        {
            l_ret.insert(a_c.first);
        }
    }

    return move(l_ret);
}

bool CAxonProxyServer::EstablishContracts(CProxyConnection& a_conn)
{
    if (not a_conn.Contracts.empty())
        return true;
    if (not a_conn.Client->IsOpen())
        return false;

    for (int i = 0; i < 3; ++i)
    {
        try
        {
            a_conn.Contracts = a_conn.Client->Send(QUERY_CONTRACTS_CONTRACT, 1000);
            return true;
        }
        catch (CDisconnectedException)
        {
            return false;
        }
        catch (CTimeoutException)
        {
            cout << (i+1) << "..." << endl;
        }
        catch (...)
        {
            return false;
        }
    }

    return false;
}

void CAxonProxyServer::usAddToOpenClients(CProxyConnection::Ptr a_conn)
{
    if (not a_conn)
        return;

    for (const string &l_c : a_conn->Contracts)
    {
        m_openClients[l_c].push_back(a_conn);
    }
}

void CAxonProxyServer::usAddToDisconnected(CProxyConnection::Ptr a_conn)
{
    if (not a_conn)
        return;

    auto l_iter = find(begin(m_closedClients), end(m_closedClients), a_conn);

    if (l_iter == end(m_closedClients))
    {
        cout << "The client " << a_conn->Client->ConnectionString() << " has disconnected. Moving to idle." << endl;

        m_closedClients.push_back(move(a_conn));
    }
}



void CAxonProxyServer::p_OnTimerTicked()
{
    // Create a copy of the closed connection list
    // so that the client lock doesn't need to be held for the duration
    // of a potentially long process
    CProxyConnectionList l_closedCopy;

    {
        lock_guard<mutex> l_lock(m_clientLock);
        l_closedCopy.insert(end(l_closedCopy),
                            begin(m_closedClients),
                            end(m_closedClients));
    }

    CProxyConnectionList l_opened;

    for (const CProxyConnection::Ptr &l_conn : l_closedCopy)
    {
        try
        {
            if (l_conn->Client->Reconnect())
            {
                cout << "Re-established connection with "
                     << l_conn->Client->ConnectionString()
                     << endl;

                if (EstablishContracts(*l_conn))
                {
                    l_opened.push_back(l_conn);
                }
            }
        }
        catch (CDisconnectedException &)
        {
            // Don't care
        }
        catch (exception &ex)
        {
            cerr << "Failed to re-establish connection to '"
                 << l_conn->Client->ConnectionString()
                 << "' because of the error: "
                 << ex.what()
                 << endl;
        }
        catch (...)
        {
            cerr << "Failed to re-establish connection to '"
                 << l_conn->Client->ConnectionString()
                 << "' because of an unknown error."
                 << endl;
        }
    }

    if (not l_opened.empty())
    {
        lock_guard<mutex> l_lock(m_clientLock);

        for (CProxyConnection::Ptr &l_conn : l_opened)
        {
            m_closedClients.erase(
                    find(begin(m_closedClients),
                         end(m_closedClients),
                         l_conn
                    )
            );

            usAddToOpenClients(move(l_conn));
        }
    }

    // Queue the next start
    m_timer.Start();
}

} }
