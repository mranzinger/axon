/*
 * File description: dispatcher.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <memory>
#include <thread>
#include <atomic>
#include <iostream>
#include <vector>

#include "dll_export.h"

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/dns.h>
#include <event2/thread.h>

#include <string.h>

#ifdef IS_WINDOWS
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

using namespace std;

namespace axon { namespace communication { namespace tcp {

typedef unique_ptr<event_base, void(*)(event_base*)> event_base_ptr;
typedef unique_ptr<evdns_base, void(*)(evdns_base*)> dns_base_ptr;
typedef unique_ptr<event, void(*)(event*)> event_ptr;

inline void FreeEventBase(event_base *a_evt)
{
	event_base_free(a_evt);
}
inline void FreeDnsBase(evdns_base *a_dns)
{
	evdns_base_free(a_dns, 1);
}
inline void FreeEvt(event *a_evt)
{
	event_free(a_evt);
}

class CDispatcher
{
private:


	struct make_private {};

	static const size_t NUM_THREADS = 8;

	vector<event_base_ptr> m_bases;
	vector<dns_base_ptr> m_dnss;
	vector<event_ptr> m_kills;
	vector<thread> m_threads;

	mutex m_lock;
	condition_variable m_startSync;

	size_t m_maxThreads;
	size_t m_lastEvt;

	bool m_terminating;

public:
	typedef shared_ptr<CDispatcher> Ptr;

	CDispatcher(size_t a_numThreads, make_private)
		: m_maxThreads(a_numThreads), m_lastEvt(0), m_terminating(false)
	{
#ifdef IS_WINDOWS
		WSADATA l_wsData{ 0 };
		WSAStartup(MAKEWORD(2, 2), &l_wsData);

		evthread_use_windows_threads();
#else
		evthread_use_pthreads();
#endif

		unique_lock<mutex> l_condLock(m_lock);

		for (size_t i = 0; i < m_maxThreads; ++i)
		{
			m_bases.emplace_back(event_base_new(), FreeEventBase);

			m_dnss.emplace_back(evdns_base_new(m_bases.back().get(), 1), FreeDnsBase);

			// Create and add an event that will cause the loop to terminate
			m_kills.emplace_back(event_new(Base(i), -1, EV_TIMEOUT | EV_PERSIST, p_TermLoop, Base(i)),
					FreeEvt);

			event_add(m_kills.back().get(), nullptr);

			m_threads.emplace_back(&CDispatcher::p_Run, this, i);

			m_startSync.wait(l_condLock);
		}
	}
	~CDispatcher()
	{
		m_terminating = true;
		for (size_t i = 0; i < NumThreads(); ++i)
		{
			// Trigger the kill event which will terminate the loop
			event_active(m_kills[i].get(), EV_TIMEOUT, 1);

			m_threads[i].join();

			event_del(m_kills[i].get());
		}
	}

	static Ptr Get(size_t a_numThreads = NUM_THREADS)
	{
		return make_shared<CDispatcher>(a_numThreads, make_private());
	}

	static size_t OptimalNumThreads()
	{
		return NUM_THREADS;
	}

	size_t NumThreads() const { return m_maxThreads; }

	event_base *GetNextBase()
	{
		unique_lock<mutex> l_lock(m_lock);

		++m_lastEvt;
		if (m_lastEvt >= m_maxThreads)
		{
			if (m_maxThreads == 1)
				m_lastEvt = 0;
			else
				m_lastEvt = 1;
		}

		return Base(m_lastEvt);
	}

	event_base *Base() const { return m_bases[0].get(); }
	event_base *Base(size_t a_idx) const { return m_bases[a_idx].get(); }
	evdns_base *Dns() const { return m_dnss[0].get(); }
	evdns_base *Dns(size_t a_idx) const { return m_dnss[a_idx].get(); }

private:
	void p_Run(size_t a_idx)
	{
		{
			lock_guard<mutex> l_lock(m_lock);
			cout << "Entering event dispatch loop " << a_idx << "." << endl;
		}
		m_startSync.notify_all();

		int l_err = 1;

		do
		{
			l_err = event_base_dispatch(m_bases[a_idx].get());
		} while (!m_terminating && l_err != -1);

		cout << "Exiting Run Function. Loop: " << a_idx << ", Status: " << l_err << endl;
	}
	static void p_TermLoop(evutil_socket_t, short, void *p)
	{
		cout << "Term Loop Invoked." << endl;

		event_base_loopexit((event_base*)p, nullptr);
	}


};

} } }

#endif /* DISPATCHER_H_ */
