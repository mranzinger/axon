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
#include <sys/socket.h>
#include <string.h>

using namespace std;

namespace axon { namespace communication { namespace tcp {

class CDispatcher
{
private:
	typedef unique_ptr<event_base, void(*)(event_base*)> event_base_ptr;
	typedef unique_ptr<evdns_base, void(*)(evdns_base*)> dns_base_ptr;

	struct make_private {};

	event_base_ptr m_base;
	dns_base_ptr m_dns;

	thread m_thread;

public:
	typedef shared_ptr<CDispatcher> Ptr;

	CDispatcher(make_private)
		: m_base(nullptr, p_FreeBase), m_dns(nullptr, p_FreeDns)
	{
		m_base.reset(event_base_new());

		m_dns.reset(evdns_base_new(Evt(), 1));

		m_thread = thread(&CDispatcher::p_Run, this);
	}
	~CDispatcher()
	{
		event_base_loopexit(m_base.get(), nullptr);

		// Terminate the loop
		event_base_once(m_base.get(), -1, EV_TIMEOUT, [](evutil_socket_t, short, void*){}, this, nullptr);

		m_thread.join();
	}

	static Ptr Get()
	{
		static weak_ptr<CDispatcher> s_weak;

		auto l_ret = s_weak.lock();

		if (l_ret)
			return l_ret;

		l_ret = make_shared<CDispatcher>(make_private());
		s_weak = l_ret;
		return l_ret;
	}

	event_base *Evt() const { return m_base.get(); }
	evdns_base *Dns() const { return m_dns.get(); }

private:
	void p_Run()
	{
		cout << "Entering event dispatch loop." << endl;

		event_base_dispatch(m_base.get());

		cout << "Exiting event dispatch loop." << endl;
	}

	static void p_FreeBase(event_base *a_evt)
	{
		event_base_free(a_evt);
	}
	static void p_FreeDns(evdns_base *a_dns)
	{
		evdns_base_free(a_dns, 1);
	}
};

} } }

#endif /* DISPATCHER_H_ */
