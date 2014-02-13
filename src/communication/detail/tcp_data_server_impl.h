/*
 * File description: tcp_data_server_impl.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef TCP_DATA_SERVER_IMPL_H_
#define TCP_DATA_SERVER_IMPL_H_

#include "communication/tcp/tcp_data_server.h"
#include "communication/tcp/tcp_data_connection.h"

#include <string>
#include <thread>
#include <atomic>

#include <event2/listener.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

using namespace std;

namespace axon { namespace communication { namespace tcp {

class CTcpDataServer::Impl
{
private:
	ConnectedHandler m_connectedHandler;
	DisconnectedHandler m_disconnectedHandler;

	CDispatcher::Ptr m_dispatcher;

public:
	Impl();
	Impl(const string &a_hostString);
	Impl(int a_port);

	void Startup(const string &a_hostString);
	void Startup(int a_port);

	string HostString() const;

	void Shutdown();

	size_t NumClients() const;

	void Broadcast(const util::CBuffer &a_buff);

	void SetConnectedHandler(ConnectedHandler a_handler);
	void SetDisconnectedHandler(DisconnectedHandler a_handler);

private:
	void p_AcceptCallback(evconnlistener *a_listener, evutil_socket_t a_sock,
			sockaddr *a_address, int a_sockLen);
	void p_AcceptErrorCallback(evconnlistener *a_listener);

	static void s_AcceptCallback(evconnlistener *a_listener, evutil_socket_t a_sock,
			sockaddr *a_address, int a_sockLen, void *a_ptr);
	static void s_AcceptErrorCallback(evconnlistener *a_listener, void *a_ptr);
};

inline CTcpDataServer::Impl::Impl()
{
	m_dispatcher = CDispatcher::Get();
}

inline CTcpDataServer::Impl::Impl(const string& a_hostString)
	: Impl()
{
}

inline CTcpDataServer::Impl::Impl(int a_port)
	: Impl()
{
}

inline void CTcpDataServer::Impl::Startup(const string& a_hostString)
{
}

inline void CTcpDataServer::Impl::Startup(int a_port)
{
}

inline string CTcpDataServer::Impl::HostString() const
{
}

inline void CTcpDataServer::Impl::Shutdown()
{
}

inline size_t CTcpDataServer::Impl::NumClients() const
{
}

inline void CTcpDataServer::Impl::Broadcast(const util::CBuffer& a_buff)
{
}

inline void CTcpDataServer::Impl::SetConnectedHandler(ConnectedHandler a_handler)
{
	m_connectedHandler = move(a_handler);
}

inline void CTcpDataServer::Impl::SetDisconnectedHandler(DisconnectedHandler a_handler)
{
	m_disconnectedHandler = move(a_handler);
}

inline void CTcpDataServer::Impl::p_AcceptCallback(evconnlistener* a_listener,
		int a_sock, sockaddr* a_address, int a_sockLen)
{
}

inline void CTcpDataServer::Impl::p_AcceptErrorCallback(
		evconnlistener* a_listener)
{
}

inline void CTcpDataServer::Impl::s_AcceptCallback(evconnlistener* a_listener,
		int a_sock, sockaddr* a_address, int a_sockLen, void* a_ptr)
{
	((Impl*)a_ptr)->p_AcceptCallback(a_listener, a_sock, a_address, a_sockLen);
}

inline void CTcpDataServer::Impl::s_AcceptErrorCallback(
		evconnlistener* a_listener, void* a_ptr)
{
	((Impl*)a_ptr)->p_AcceptErrorCallback(a_listener);
}

}
}
}



#endif /* TCP_DATA_SERVER_IMPL_H_ */
