/*
 * File description: dispatcher.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <memory>
#include <thread>
#include <atomic>
#include <iostream>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/dns.h>
#include <event2/thread.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

using namespace std;

namespace axon { namespace communication { namespace tcp {

class CDispatcher
{
private:
	typedef unique_ptr<event_base, void(*)(event_base*)> event_base_ptr;
	typedef unique_ptr<evdns_base, void(*)(evdns_base*)> dns_base_ptr;
	typedef unique_ptr<event, void(*)(event*)> event_ptr;

	struct make_private {};

	event_base_ptr m_base;
	dns_base_ptr m_dns;

	thread m_thread;

public:
	typedef shared_ptr<CDispatcher> Ptr;

	CDispatcher(make_private)
		: m_base(nullptr, p_FreeBase), m_dns(nullptr, p_FreeDns)
	{
		evthread_use_pthreads();

		m_base.reset(event_base_new());

		m_dns.reset(evdns_base_new(Evt(), 1));

		m_thread = thread(&CDispatcher::p_Run, this);
	}
	~CDispatcher()
	{
		timeval tfast{0,10};

		event_ptr l_timeout(event_new(m_base.get(), -1, 0,
				p_TermLoop, this), p_FreeEvt);

		// Tell the running event loop to exit once it finishes
		//event_base_loopexit(m_base.get(), nullptr);

		event_add(l_timeout.get(), &tfast);
		//event_add(l_timeout.get(), nullptr);

		// Make the event start
		event_active(l_timeout.get(), EV_TIMEOUT, 1);

		//m_thread.detach();
		m_thread.join();
	}

	static Ptr Get()
	{
		static Ptr s_ptr = make_shared<CDispatcher>(make_private());

		return s_ptr;
	}

	event_base *Evt() const { return m_base.get(); }
	evdns_base *Dns() const { return m_dns.get(); }

private:
	void p_Run()
	{
		cout << "Entering event dispatch loop." << endl;

		int l_err = event_base_dispatch(m_base.get());

		cout << "Exiting event dispatch loop. Status: " << l_err << endl;
	}
	static void p_TermLoop(evutil_socket_t, short, void *p)
	{
		cout << "Term Loop Invoked." << endl;

		//event_base_loopbreak(((CDispatcher*)p)->m_base.get());
		event_base_loopexit(((CDispatcher*)p)->m_base.get(), nullptr);
	}

	static void p_FreeBase(event_base *a_evt)
	{
		event_base_free(a_evt);
	}
	static void p_FreeDns(evdns_base *a_dns)
	{
		evdns_base_free(a_dns, 1);
	}
	static void p_FreeEvt(event *a_evt)
	{
		event_free(a_evt);
	}
};

} } }

#endif /* DISPATCHER_H_ */
