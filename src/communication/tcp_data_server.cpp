/*
 * File description: tcp_data_server.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include "communication/tcp/tcp_data_server.h"

#include "detail/tcp_data_server_impl.h"

namespace axon { namespace communication { namespace tcp {

namespace {

register_server<CTcpDataServer> s_tcpRegister("tcp");

}

CTcpDataServer::CTcpDataServer()
	: m_impl(new Impl)
{
}

CTcpDataServer::CTcpDataServer(const string &a_hostString)
	: m_impl(new Impl(a_hostString))
{
}

CTcpDataServer::CTcpDataServer(int a_port)
	: m_impl(new Impl(a_port))
{
}

CTcpDataServer::~CTcpDataServer()
{
	// Marker destructor that enables the opaque Impl pointer
	// to be managed by unique_ptr
}

void CTcpDataServer::Startup(const string& a_hostString)
{
	m_impl->Startup(a_hostString);
}

void CTcpDataServer::Startup(int a_port)
{
	m_impl->Startup(a_port);
}

std::string CTcpDataServer::HostString() const
{
	return m_impl->HostString();
}

void CTcpDataServer::Shutdown()
{
	m_impl->Shutdown();
}

size_t CTcpDataServer::NumClients() const
{
	return m_impl->NumClients();
}

void CTcpDataServer::Broadcast(const util::CBuffer& a_buff)
{
	m_impl->Broadcast(a_buff);
}

void CTcpDataServer::SetConnectedHandler(ConnectedHandler a_handler)
{
	m_impl->SetConnectedHandler(a_handler);
}

void CTcpDataServer::SetDisconnectedHandler(DisconnectedHandler a_handler)
{
	m_impl->SetDisconnectedHandler(a_handler);
}


}
}
}

