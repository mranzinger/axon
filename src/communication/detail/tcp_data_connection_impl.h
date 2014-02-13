/*
 * File description: tcp_data_connection_impl.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef TCP_DATA_CONNECTION_IMPL_H_
#define TCP_DATA_CONNECTION_IMPL_H_

#include "communication/tcp/tcp_data_connection.h"

#include "dispatcher.h"

#include "util/string_convert.h"

using namespace std;
using namespace axon::util;

namespace axon { namespace communication { namespace tcp {

class CTcpDataConnection::Impl
{
private:
	friend class CDispatcher;

	typedef unique_ptr<bufferevent, void (*)(bufferevent*)> bufferevent_ptr;
	bufferevent_ptr m_evt;

	CDispatcher::Ptr m_disp;

	string m_hostName;
	int m_port;

	DataReceivedHandler m_rcvHandler;

	condition_variable *m_var;

	atomic<bool> m_open;

public:
	virtual ~Impl() { }

	Impl();
	Impl(const string &a_connectionString);
	Impl(string a_hostName, int a_port);


	string ConnectionString() const;
	bool Connect(string a_hostName, int a_port);
	bool Connect(const string &a_connectionString);
	void Close();
	bool IsOpen() const;

	void Send(const CBuffer &a_buff, condition_variable *a_finishEvt);

	void SetReceiveHandler(DataReceivedHandler a_handler);

private:
	void p_WriteCallback(bufferevent *a_evt);
	void p_ReadCallback(bufferevent *a_evt);
	void p_EventCallback(bufferevent *a_evt, short a_flags);

	static void s_WriteCallback(bufferevent *a_evt, void *a_ptr);
	static void s_ReadCallback(bufferevent *a_evt, void *a_ptr);
	static void s_EventCallback(bufferevent *a_evt, short a_flags, void *a_ptr);

	static void p_FreeBuffEvt(bufferevent *a_evt)
	{
		bufferevent_free(a_evt);
	}
};



inline CTcpDataConnection::Impl::Impl()
	: m_port(-1), m_var(nullptr), m_open(false), m_evt(nullptr, p_FreeBuffEvt)
{
	m_disp = CDispatcher::Get();

	auto l_evt = bufferevent_socket_new(m_disp->Evt(), -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
	m_evt.reset(l_evt);

	bufferevent_setcb(m_evt.get(), s_ReadCallback, s_WriteCallback, s_EventCallback, this);
	bufferevent_enable(m_evt.get(), EV_READ|EV_WRITE);
}

inline CTcpDataConnection::Impl::Impl(const string& a_connectionString)
	: Impl()
{
	if (!Connect(a_connectionString))
		throw runtime_error("Unable to connect to the specified endpoint.");
}

inline CTcpDataConnection::Impl::Impl(string a_hostName, int a_port)
	: Impl()
{
	if (!Connect(move(a_hostName), a_port))
		throw runtime_error("Unable to connect to the specified endpoint.");
}

inline string CTcpDataConnection::Impl::ConnectionString() const
{
	return m_hostName + ":" + ToString(m_port);
}

inline bool CTcpDataConnection::Impl::Connect(const string& a_connectionString)
{
	size_t l_colIdx = a_connectionString.find(':');

	if (l_colIdx == string::npos)
		throw runtime_error("Invalid TCP connection string. Must be of format [host name]:[port]");

	string l_hostName = a_connectionString.substr(0, l_colIdx);

	int l_port = StringTo<int>(a_connectionString.substr(l_colIdx + 1));

	return Connect(move(l_hostName), l_port);
}

inline bool CTcpDataConnection::Impl::Connect(string a_hostName, int a_port)
{
	int l_err = bufferevent_socket_connect_hostname(m_evt.get(), m_disp->Dns(),
			AF_UNSPEC, a_hostName.c_str(), a_port);

	if (l_err)
		return false;

	m_hostName = move(a_hostName);
	m_port = a_port;

	return true;
}

inline void CTcpDataConnection::Impl::Close()
{
	// TODO: Figure out how to do this
}

inline bool CTcpDataConnection::Impl::IsOpen() const
{
	return m_open;
}

inline void CTcpDataConnection::Impl::Send(const CBuffer& a_buff, condition_variable* a_finishEvt)
{
	m_var = a_finishEvt;

	bufferevent_write(m_evt.get(), a_buff.data(), a_buff.size());
}

inline void CTcpDataConnection::Impl::SetReceiveHandler(DataReceivedHandler a_handler)
{
	m_rcvHandler = move(a_handler);
}

inline void CTcpDataConnection::Impl::p_WriteCallback(bufferevent* a_evt)
{
	// TODO: Determine if the write finished
}

inline void CTcpDataConnection::Impl::p_ReadCallback(bufferevent* a_evt)
{
	evbuffer *l_input = bufferevent_get_input(a_evt);

	CDataBuffer l_buff;

	bool l_reAlloc = true;

	char *l_dest = l_buff.data();
	char *l_end = l_buff.end();

	int n = 0;
	do
	{
		if (l_reAlloc)
		{
			size_t l_len = evbuffer_get_length(l_input);

			if (l_len == 0)
				break;

			l_buff.Reset(l_len);
			l_dest = l_buff.data();
			l_end = l_buff.data();

			l_reAlloc = false;
		}

		n = evbuffer_remove(l_input, l_dest, l_end - l_dest);

		l_dest += n;

		if (l_dest == l_end)
		{
			m_rcvHandler(move(l_buff));
			l_reAlloc = true;
		}

	} while (n > 0);

	if (l_buff.size() > 0)
		m_rcvHandler(move(l_buff));
}

inline void CTcpDataConnection::Impl::p_EventCallback(bufferevent* a_evt, short a_flags)
{
	bool l_close = false;

	if (a_flags & BEV_EVENT_CONNECTED)
	{
		m_open = true;
	}
	else if (a_flags & BEV_EVENT_ERROR)
	{
		l_close = true;
		// TODO: Log
	}
	else if (a_flags & BEV_EVENT_EOF)
	{
		l_close = true;
	}

	if (l_close)
	{
		m_open = false;
	}
}

inline void CTcpDataConnection::Impl::s_WriteCallback(bufferevent* a_evt, void* a_ptr)
{
	((Impl*)a_ptr)->p_WriteCallback(a_evt);
}

inline void CTcpDataConnection::Impl::s_ReadCallback(bufferevent* a_evt, void* a_ptr)
{
	((Impl*)a_ptr)->p_ReadCallback(a_evt);
}

inline void CTcpDataConnection::Impl::s_EventCallback(bufferevent* a_evt, short a_flags, void* a_ptr)
{
	((Impl*)a_ptr)->p_EventCallback(a_evt, a_flags);
}

}
}
}



#endif /* TCP_DATA_CONNECTION_IMPL_H_ */
