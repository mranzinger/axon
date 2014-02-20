/*
 * File description: tcp_data_server_impl.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef TCP_DATA_SERVER_IMPL_H_
#define TCP_DATA_SERVER_IMPL_H_

#include "communication/tcp/tcp_data_server.h"
#include "communication/tcp/tcp_data_connection.h"

#include "tcp_data_connection_impl.h"

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>

#include <event2/listener.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#ifdef IS_WINDOWS
#include <WinSock2.h>
#include <Ws2tcpip.h>

#define inet_ntop InetNtop
#else
#include <arpa/inet.h>
#endif

#include "util/string_convert.h"

using namespace std;
using namespace axon::util;

namespace axon { namespace communication { namespace tcp {

class CServerConnImpl;

class CTcpDataServer::Impl
{
private:
	friend class CServerConnImpl;

	typedef unique_ptr<evconnlistener, void(*)(evconnlistener*)> evconnlistener_ptr;
	evconnlistener_ptr m_listener;

	ConnectedHandler m_connectedHandler;
	DisconnectedHandler m_disconnectedHandler;

	CDispatcher::Ptr m_dispatcher;

	mutable mutex m_connLock;
	unordered_map<CTcpDataConnection*, CTcpDataConnection::Ptr> m_conns;

	int m_port;

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
	static void s_FreeListener(evconnlistener *a_listener);
};

inline CTcpDataServer::Impl::Impl()
	: m_port(-1), m_listener(nullptr, s_FreeListener)
{
	m_dispatcher = CDispatcher::Get();
}

inline CTcpDataServer::Impl::Impl(const string& a_hostString)
	: Impl()
{
	Startup(a_hostString);
}

inline CTcpDataServer::Impl::Impl(int a_port)
	: Impl()
{
	Startup(a_port);
}

inline void CTcpDataServer::Impl::Startup(const string& a_hostString)
{
	Startup(StringTo<int>(a_hostString));
}

inline void CTcpDataServer::Impl::Startup(int a_port)
{
	sockaddr_in l_in;
	memset(&l_in, 0, sizeof(l_in));
	l_in.sin_family = AF_INET;
	l_in.sin_addr.s_addr = htonl(INADDR_ANY);
	l_in.sin_port = htons(a_port);

	m_listener.reset(
			evconnlistener_new_bind(m_dispatcher->Evt(), s_AcceptCallback, this, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
					(sockaddr*)&l_in, sizeof(l_in))
	);
	evconnlistener_set_error_cb(m_listener.get(), s_AcceptErrorCallback);
}

inline string CTcpDataServer::Impl::HostString() const
{
	return ToString(m_port);
}

inline void CTcpDataServer::Impl::Shutdown()
{
	evconnlistener_disable(m_listener.get());
}

inline size_t CTcpDataServer::Impl::NumClients() const
{
	lock_guard<mutex> l_lock(m_connLock);
	return m_conns.size();
}

inline void CTcpDataServer::Impl::Broadcast(const util::CBuffer& a_buff)
{
	lock_guard<mutex> l_lock(m_connLock);
	for (const auto &iter : m_conns)
	{
		iter.first->Send(a_buff, nullptr);
	}
}

inline void CTcpDataServer::Impl::SetConnectedHandler(ConnectedHandler a_handler)
{
	m_connectedHandler = move(a_handler);
}

inline void CTcpDataServer::Impl::SetDisconnectedHandler(DisconnectedHandler a_handler)
{
	m_disconnectedHandler = move(a_handler);
}


class CServerConnImpl
	: public CTcpDataConnection::Impl
{
private:
	CTcpDataServer::Impl *m_server;

public:
	CServerConnImpl(CTcpDataServer::Impl *a_server, bufferevent_ptr a_evt,
			string a_hostName, int a_port)
		: CTcpDataConnection::Impl(move(a_evt), move(a_hostName), a_port),
		  m_server(a_server)
	{

	}

	CTcpDataConnection *Conn = nullptr;

	virtual bool Connect(string a_hostName, int a_port) override { p_ThrowInvalid(); return false; }
	virtual bool Connect(const string &a_connectionString) override { p_ThrowInvalid(); return false; }
	virtual void Close() override
	{
		CTcpDataConnection::Impl::Close();

		IDataConnection::Ptr l_conn;

		{
			lock_guard<mutex> l_lock(m_server->m_connLock);

			auto iter = m_server->m_conns.find(Conn);

			if (iter == m_server->m_conns.end())
				return;

			l_conn = iter->second;
			m_server->m_conns.erase(iter);
		}

		cout << "Cient Disconnected." << endl;

		m_server->m_disconnectedHandler(l_conn);
	}
	virtual bool IsServerClient() const override { return true; }

private:
	void p_ThrowInvalid()
	{
		throw runtime_error("The specified operation is not permitted on server managed connections.");
	}
};

inline void CTcpDataServer::Impl::p_AcceptCallback(evconnlistener* a_listener,
		evutil_socket_t a_sock, sockaddr* a_address, int a_sockLen)
{
	char l_scratch[INET6_ADDRSTRLEN];
	inet_ntop(a_address->sa_family, a_address->sa_data, l_scratch, sizeof(l_scratch));

	int l_port;
	if (a_address->sa_family == AF_INET)
		l_port = ((sockaddr_in*)a_address)->sin_port;
	else
		l_port = ((sockaddr_in6*)a_address)->sin6_port;

	bufferevent_ptr l_evt(
			bufferevent_socket_new(evconnlistener_get_base(a_listener),
				a_sock, BEV_OPT_CLOSE_ON_FREE),
			s_FreeBuffEvt
	);

	std::unique_ptr<CServerConnImpl> l_impl(new CServerConnImpl(this, move(l_evt), l_scratch, l_port));

	auto l_ptr = l_impl.get();

	auto l_conn = make_shared<CTcpDataConnection>(move(l_impl));
	l_ptr->Conn = l_conn.get();

	{
		lock_guard<mutex> l_lock(m_connLock);
		m_conns.insert(make_pair(l_conn.get(), l_conn));
	}

	cout << "Client Connected." << endl;

	m_connectedHandler(l_conn);
}

inline void CTcpDataServer::Impl::p_AcceptErrorCallback(
		evconnlistener* a_listener)
{
}

inline void CTcpDataServer::Impl::s_AcceptCallback(evconnlistener* a_listener,
		evutil_socket_t a_sock, sockaddr* a_address, int a_sockLen, void* a_ptr)
{
	((Impl*)a_ptr)->p_AcceptCallback(a_listener, a_sock, a_address, a_sockLen);
}

inline void CTcpDataServer::Impl::s_AcceptErrorCallback(
		evconnlistener* a_listener, void* a_ptr)
{
	((Impl*)a_ptr)->p_AcceptErrorCallback(a_listener);
}

inline void CTcpDataServer::Impl::s_FreeListener(evconnlistener* a_listener)
{
	evconnlistener_free(a_listener);
}

}
}
}



#endif /* TCP_DATA_SERVER_IMPL_H_ */
