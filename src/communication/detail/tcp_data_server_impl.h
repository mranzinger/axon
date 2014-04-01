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
#include <algorithm>

#include <event2/listener.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#ifdef IS_WINDOWS
#include <WinSock2.h>
#include <Ws2tcpip.h>

#define inet_ntop InetNtop

#undef min
#undef max
#else
#include <arpa/inet.h>
#endif

#include "util/string_convert.h"
#include "util/simple_math.h"

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

	mutex m_rebalanceLock;
	high_resolution_clock::time_point m_lastRebalance;
	typedef pair<event_base*,size_t> base_timing;
	vector<base_timing> m_timers;

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

	void UpdateClientProcTime(CServerConnImpl *a_client, const dirus &a_dur);

private:
	void p_AcceptCallback(evconnlistener *a_listener, evutil_socket_t a_sock,
			sockaddr *a_address, int a_sockLen);
	void p_AcceptErrorCallback(evconnlistener *a_listener);
	void p_DoRebalance();

	static void s_AcceptCallback(evconnlistener *a_listener, evutil_socket_t a_sock,
			sockaddr *a_address, int a_sockLen, void *a_ptr);
	static void s_AcceptErrorCallback(evconnlistener *a_listener, void *a_ptr);
	static void s_FreeListener(evconnlistener *a_listener);
	static void s_DoRebalance(evutil_socket_t, short, void *p);
};

class CServerConnImpl
	: public CTcpDataConnection::Impl
{
private:
	CTcpDataServer::Impl *m_server;

public:
	CServerConnImpl(CTcpDataServer::Impl *a_server, string a_hostName, int a_port, CDispatcher::Ptr a_dispatcher)
		: CTcpDataConnection::Impl(move(a_hostName), a_port, a_dispatcher),
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

		cout << "Client Disconnected." << endl;

		m_server->m_disconnectedHandler(l_conn);
	}
	virtual bool IsServerClient() const override { return true; }

	void EstablishEvt(bufferevent_ptr a_evt)
	{
		p_SetBufferEvent(move(a_evt));
	}

	size_t GetProcTime() const
	{
		return m_procTime;
	}

	event_base *GetBase() const
	{
		return bufferevent_get_base(GetBufferEvent());
	}

	virtual void UpdateProcTime(const dirus& a_dur) override
	{
		CTcpDataConnection::Impl::UpdateProcTime(a_dur);

		m_server->UpdateClientProcTime(this, a_dur);
	}

private:
	void p_ThrowInvalid()
	{
		throw runtime_error("The specified operation is not permitted on server managed connections.");
	}
};

inline CTcpDataServer::Impl::Impl()
	: m_port(-1), m_listener(nullptr, s_FreeListener)
{
	m_dispatcher = CDispatcher::Get(CDispatcher::OptimalNumThreads() + 1);

	m_lastRebalance = high_resolution_clock::time_point::min();

	for (size_t i = 0; i < CDispatcher::OptimalNumThreads(); ++i)
	{
		m_timers.emplace_back(m_dispatcher->Base(i+1), 0);
	}
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
			evconnlistener_new_bind(m_dispatcher->Base(0), s_AcceptCallback, this, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
					(sockaddr*)&l_in, sizeof(l_in))
	);

	if (!m_listener)
		throw runtime_error("Failed to bind server to the specified port. Verify that the port is not already in use.");

	evconnlistener_set_error_cb(m_listener.get(), s_AcceptErrorCallback);
}

inline string CTcpDataServer::Impl::HostString() const
{
	return ToString(m_port);
}

inline void CTcpDataServer::Impl::Shutdown()
{
	lock_guard<mutex> l_lock(m_connLock);
	m_conns.clear();
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

	std::unique_ptr<CServerConnImpl> l_impl(new CServerConnImpl(this, l_scratch, l_port, m_dispatcher));

	auto l_ptr = l_impl.get();

	auto l_conn = make_shared<CTcpDataConnection>(move(l_impl));
	l_ptr->Conn = l_conn.get();

	{
		lock_guard<mutex> l_lock(m_connLock);
		m_conns.insert(make_pair(l_conn.get(), l_conn));
	}

	cout << "Client Connected." << endl;

	if (m_connectedHandler)
		m_connectedHandler(l_conn);

	event_base *l_baseHandler = m_dispatcher->GetNextBase();

	bufferevent_ptr l_evt(
			bufferevent_socket_new(l_baseHandler, a_sock, BEV_OPT_CLOSE_ON_FREE),
			s_FreeBuffEvt
	);

	l_ptr->EstablishEvt(move(l_evt));
}

struct RBPair
{
	event_ptr Evt;
	CTcpDataServer::Impl *Impl;

	RBPair() : Evt(nullptr, FreeEvt), Impl(nullptr) { }
};

inline void CTcpDataServer::Impl::UpdateClientProcTime(CServerConnImpl* a_client, const dirus& a_dur)
{
	event_base *l_currBase = a_client->GetBase();

	auto l_timing = find_if(m_timers.begin(), m_timers.end(),
			[l_currBase] (const base_timing &a_timing)
			{
				return l_currBase == a_timing.first;
			});

	if (l_timing == m_timers.end())
	{
		cerr << "Unable to update detached client timing." << endl;
		// This really shouldn't happen, but not really a big deal
		return;
	}

	// Update the number of microseconds spent processing on this event_base
	l_timing->second += a_dur.count();

	auto l_currTime = high_resolution_clock::now();

	if (m_lastRebalance == high_resolution_clock::time_point::min())
	{
		m_lastRebalance = l_currTime;
		return;
	}

    // Rebalancing temporarily disabled
	// Re-balance every 10 seconds
	/*if ((l_currTime - m_lastRebalance) < seconds(10))
		return;

	unique_lock<mutex> l_lock(m_rebalanceLock, try_to_lock);

	// Obviously only allow one thread to be rebalancing
	if (!l_lock.owns_lock())
		return;

	m_lastRebalance = l_currTime;

	// Create a new event that will run on the listener thread.
	RBPair *l_rb = new RBPair;
	l_rb->Impl = this;

	event_ptr l_runPtr{
		event_new(m_dispatcher->Base(0), -1, EV_TIMEOUT | EV_PERSIST, s_DoRebalance, l_rb),
		FreeEvt
	};

	l_rb->Evt = move(l_runPtr);

	event_add(l_rb->Evt.get(), nullptr);

	event_active(l_rb->Evt.get(), EV_TIMEOUT, 1);*/
}

inline void CTcpDataServer::Impl::p_DoRebalance()
{
    // Rebalancing temporarily disabled
    return;

#ifdef AXON_VERBOSE
	cout << endl;
	cout << "Executing Load Re-balance Routine" << endl;
#endif

	// Create a copy of the timings. This is so that the math isn't unstable
	vector<double> l_times;
	for (const base_timing &l_base : m_timers)
		l_times.push_back(l_base.second);

	double l_mean = Mean(l_times.begin(), l_times.end());
	double l_stdDev = StdDev(l_times.begin(), l_times.end());

	size_t i = 0;
#ifdef AXON_VERBOSE
	cout << "Average Handler Time: " << duration_cast<milliseconds>(microseconds(size_t(l_mean))).count() << "ms" << endl
		 << "Standard Deviation: " << duration_cast<milliseconds>(microseconds(size_t(l_stdDev))).count() << "ms" << endl << endl
		 << "Individual Threads:" << endl;
	for (double l_time : l_times)
	{
		cout << "(" << i++ << ") " << duration_cast<milliseconds>(microseconds(size_t(l_time))).count() << "ms" << endl;
	}
#endif

	vector<size_t> l_busyThreads;
	for (i = 0; i < l_times.size(); ++i)
	{
		double l_micro = l_times[i];

		if (l_micro > l_mean + l_stdDev)
			l_busyThreads.push_back(i);
	}

#ifdef AXON_VERBOSE
	cout << endl;

	cout << "Busy Threads: ";
	for_each(l_busyThreads.begin(), l_busyThreads.end(), [](size_t i) { cout << i << " "; });
	cout << endl << endl;
#endif

	for (size_t l_busy : l_busyThreads)
	{
#ifdef AXON_VERBOSE
		cout << "Attempting to balance thread " << l_busy << endl;
#endif

		vector<CServerConnImpl*> l_busyClients;
		vector<double> l_busyTimes;

		for (const auto &l_pair : m_conns)
		{
			auto impl = (CServerConnImpl*)l_pair.second->GetImpl();

			if (impl->GetBase() == m_dispatcher->Base(l_busy + 1))
			{
				l_busyClients.push_back(impl);
				l_busyTimes.push_back(duration_cast<milliseconds>(microseconds(impl->GetProcTime())).count());
			}
		}

		if (l_busyClients.empty())
			continue;

#ifdef AXON_VERBOSE
		for (double c : l_busyTimes)
		{
			cout << " - " << c << endl;
		}
#endif

		size_t l_from = rand() % l_busyClients.size();

		auto l_min = FindMin(l_times.begin(), l_times.end());

		size_t l_to = l_min - l_times.begin();

#ifdef AXON_VERBOSE
		cout << "Moving client " << l_from << " to thread " << l_to << endl;
#endif
		bufferevent_base_set(m_dispatcher->Base(l_to + 1), l_busyClients[l_from]->GetBufferEvent());

		l_times[l_busy] -= l_busyTimes[l_from];
		*l_min += l_busyTimes[l_from];

#ifdef AXON_VERBOSE
		cout << endl;
#endif
	}

	for (base_timing &l_base : m_timers)
		l_base.second = 0;

	for (auto &l_pair : m_conns)
		l_pair.second->GetImpl()->ResetProcTime();

#ifdef AXON_VERBOSE
	cout << endl;
#endif
}

inline void CTcpDataServer::Impl::p_AcceptErrorCallback(evconnlistener* a_listener)
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

inline void CTcpDataServer::Impl::s_DoRebalance(evutil_socket_t,
		short shortInt, void* p)
{
	RBPair *l_rb = (RBPair*)p;

	try
	{
		l_rb->Impl->p_DoRebalance();
		delete l_rb;
	}
	catch (...)
	{
		delete l_rb;
		throw;
	}
}

}
}
}



#endif /* TCP_DATA_SERVER_IMPL_H_ */
