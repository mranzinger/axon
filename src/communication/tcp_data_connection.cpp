/*
 * File description: tcp_data_connection.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include "communication/tcp/tcp_data_connection.h"

#include "detail/tcp_data_connection_impl.h"

using namespace std;

namespace axon { namespace communication { namespace tcp {

namespace {

register_protocol<CTcpDataConnection> s_tcpRegister("tcp");

}

CTcpDataConnection::CTcpDataConnection()
	: m_impl(new Impl)
{
}

CTcpDataConnection::CTcpDataConnection(const std::string& a_connectionString)
	: m_impl(new Impl(a_connectionString))
{
}

CTcpDataConnection::CTcpDataConnection(std::string a_hostName, int a_port)
	: m_impl(new Impl(move(a_hostName), a_port))
{
}

CTcpDataConnection::CTcpDataConnection(std::unique_ptr<Impl> a_impl)
	: m_impl(move(a_impl))
{
}

CTcpDataConnection::Ptr CTcpDataConnection::Create()
{
    return make_shared<CTcpDataConnection>();
}

CTcpDataConnection::Ptr CTcpDataConnection::Create(string a_hostName, int a_port)
{
    return make_shared<CTcpDataConnection>(move(a_hostName), a_port);
}

CTcpDataConnection::~CTcpDataConnection()
{
	// Destructor simply here so that the unique_ptr to an opaque class
	// will compile
}

std::string CTcpDataConnection::ConnectionString() const
{
	return m_impl->ConnectionString();
}

bool CTcpDataConnection::Connect(std::string a_hostName, int a_port)
{
	return m_impl->Connect(move(a_hostName), a_port);
}

bool CTcpDataConnection::Connect(const std::string& a_connectionString)
{
	return m_impl->Connect(a_connectionString);
}

bool CTcpDataConnection::Reconnect()
{
    return m_impl->Reconnect();
}

void CTcpDataConnection::Close()
{
	m_impl->Close();
}

bool CTcpDataConnection::IsOpen() const
{
	return m_impl->IsOpen();
}

bool CTcpDataConnection::IsServerClient() const
{
	return m_impl->IsServerClient();
}



void CTcpDataConnection::Send(const util::CBuffer& a_buff, std::condition_variable* a_finishEvt)
{
	m_impl->Send(a_buff, a_finishEvt);
}

void CTcpDataConnection::SetReceiveHandler(DataReceivedHandler a_handler)
{
	m_impl->SetReceiveHandler(move(a_handler));
}

}
}
}


