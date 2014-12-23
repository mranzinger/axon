/*
 * File description: tcp_data_connection_impl.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef TCP_DATA_CONNECTION_IMPL_H_
#define TCP_DATA_CONNECTION_IMPL_H_

#include <chrono>

#include "communication/tcp/tcp_data_connection.h"
#include "communication/disconnected_exception.h"

#include "dispatcher.h"

#include "util/string_convert.h"

using namespace std;
using namespace std::chrono;
using namespace axon::util;

namespace axon { namespace communication { namespace tcp {

typedef unique_ptr<bufferevent, void (*)(bufferevent*)> bufferevent_ptr;

typedef microseconds dirus;

inline void s_FreeBuffEvt(bufferevent *a_evt)
{
	if (a_evt)
		bufferevent_free(a_evt);
}

class CTcpDataConnection::Impl
{
private:
	friend class CDispatcher;

	bufferevent_ptr m_evt;

	CDispatcher::Ptr m_disp;

	string m_hostName;
	int m_port;

	DataReceivedHandler m_rcvHandler;

	//condition_variable *m_var;

	atomic<bool> m_open;
	atomic<bool> m_error;
	atomic<bool> m_connecting;
	recursive_mutex m_openLock;
	condition_variable_any m_openCV;

	//mutex m_sendLock;

protected:
	atomic<size_t> m_procTime;

public:
	virtual ~Impl();

	Impl();
	Impl(const string &a_connectionString);
	Impl(string a_hostName, int a_port);


	string ConnectionString() const;
	virtual bool Connect(string a_hostName, int a_port);
	virtual bool Connect(const string &a_connectionString);
	virtual bool Reconnect();
	virtual void Close();
	bool IsOpen() const;
	virtual bool IsServerClient() const { return false; }

	void Send(const CBuffer &a_buff, condition_variable *a_finishEvt);

	void SetReceiveHandler(DataReceivedHandler a_handler);

	bufferevent *GetBufferEvent() const { return m_evt.get(); }

	size_t GetProcTime() const { return m_procTime; }

	void ResetProcTime() { m_procTime = size_t(0); }

protected:
	Impl(string a_hostName, int a_port, CDispatcher::Ptr a_dispatcher);

	void p_SetBufferEvent(bufferevent_ptr a_evt);

	virtual void UpdateProcTime(const dirus &a_dur);



private:
	void p_ResetEvt();
	void p_WriteCallback(bufferevent *a_evt);
	void p_ReadCallback(bufferevent *a_evt);
	void p_EventCallback(bufferevent *a_evt, short a_flags);
	void p_HookupEvt();
	void p_UnhookEvt();

	static void s_WriteCallback(bufferevent *a_evt, void *a_ptr);
	static void s_ReadCallback(bufferevent *a_evt, void *a_ptr);
	static void s_EventCallback(bufferevent *a_evt, short a_flags, void *a_ptr);
};



inline CTcpDataConnection::Impl::Impl()
	: m_port(-1), m_open(false), m_connecting(false), m_error(false), m_evt(nullptr, s_FreeBuffEvt), m_procTime(0)
{
	m_disp = CDispatcher::Get(1);

	//p_ResetEvt();
}

inline CTcpDataConnection::Impl::Impl(const string& a_connectionString)
	: Impl()
{
	Connect(a_connectionString);
}

inline CTcpDataConnection::Impl::Impl(string a_hostName, int a_port)
	: Impl()
{
	Connect(move(a_hostName), a_port);
}

inline CTcpDataConnection::Impl::Impl(string a_hostName, int a_port, CDispatcher::Ptr a_dispatcher)
	: m_port(a_port), m_open(true), m_connecting(false), m_error(false), m_hostName(move(a_hostName)),
	  m_evt(nullptr, s_FreeBuffEvt),
	  m_disp(move(a_dispatcher)), m_procTime(0)
{
}

inline CTcpDataConnection::Impl::~Impl()
{
    p_UnhookEvt();
}

inline void CTcpDataConnection::Impl::p_SetBufferEvent(bufferevent_ptr a_evt)
{
	m_evt = move(a_evt);

	p_HookupEvt();
}

inline void CTcpDataConnection::Impl::p_ResetEvt()
{
    auto l_evt = bufferevent_socket_new(m_disp->Base(), -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
    m_evt.reset(l_evt);

    p_HookupEvt();
}

inline void CTcpDataConnection::Impl::p_HookupEvt()
{
    if (not m_evt)
        return;

	bufferevent_setcb(m_evt.get(), s_ReadCallback, s_WriteCallback, s_EventCallback, this);
	bufferevent_enable(m_evt.get(), EV_READ|EV_WRITE);
}

inline void CTcpDataConnection::Impl::p_UnhookEvt()
{
    if (not m_evt)
        return;

    bufferevent_disable(m_evt.get(), EV_READ|EV_WRITE);
    bufferevent_setcb(m_evt.get(), nullptr, nullptr, nullptr, nullptr);
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
	unique_lock<recursive_mutex> l_lock(m_openLock);

	m_hostName = move(a_hostName);
    m_port = a_port;

	if (not m_connecting)
	{
	    m_connecting = true;
	    m_error = false;

	    p_ResetEvt();

	    int l_err = bufferevent_socket_connect_hostname(m_evt.get(), m_disp->Dns(),
            AF_UNSPEC, a_hostName.c_str(), a_port);

	    if (l_err)
	    {
	        m_connecting = false;
	        return false;
	    }
	}

	// There is apparently a condition in which the connect method will not be invoked
	// in a separate thread, which means that the status will be updated during the call
	// to connect. In this case, we definitely do not want to wait for the condition
	// because it was signalled before even reaching this point
	if (m_connecting)
	{
	    // Wait for up to 10 seconds for the connection to open
	    /*cv_status l_status = m_openCV.wait(l_lock, chrono::seconds(10));

	    if (l_status == cv_status::timeout)
	        return false;*/
	    m_openCV.wait(l_lock);
	}

	return m_open;
}

inline bool CTcpDataConnection::Impl::Reconnect()
{
    if (m_open)
    {
        Close();
    }

    return Connect(m_hostName, m_port);
}

inline void CTcpDataConnection::Impl::Close()
{
	m_open = false;

	/*bufferevent_free(m_evt.get());
	m_evt.reset();

	p_ResetEvt();*/

	/*int fd = bufferevent_getfd(m_evt.get());

	if (fd < 0)
	    return;

	int err = evutil_closesocket(fd);
	if (err != 0)
	{
	    auto err = evutil_socket_geterror(fd);

	    auto str = evutil_socket_error_to_string(err);

	    cerr << "Failed to close the socket for "
	         << ConnectionString()
	         << ". Error (" << err << "): " << str << endl;
	}*/
}



inline bool CTcpDataConnection::Impl::IsOpen() const
{
	return m_open && m_evt;
}

inline void CTcpDataConnection::Impl::Send(const CBuffer& a_buff, condition_variable* a_finishEvt)
{
	if (a_finishEvt)
	    throw runtime_error("Signaling the end of the send is not currently supported.");

	if (not IsOpen())
	    throw CDisconnectedException();

	int l_ret;
	{
	    //lock_guard<mutex> l_lock(m_sendLock);

	    evbuffer *l_output = bufferevent_get_output(m_evt.get());

	    evbuffer_lock(l_output);

	    try
	    {
	        l_ret = bufferevent_write(m_evt.get(), a_buff.data(), a_buff.size());

	        evbuffer_unlock(l_output);
	    }
	    catch (...)
	    {
	        evbuffer_unlock(l_output);
	        throw;
	    }
	}

	if (l_ret != 0)
		cout << "Failed to write socket data." << endl;
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
	auto l_start = high_resolution_clock::now();

	evbuffer *l_input = bufferevent_get_input(a_evt);

	const size_t l_size = 4096;
	CDataBuffer l_buff(l_size);

	bool l_reAlloc = true;

	evbuffer_lock(l_input);

	try
	{
        while (true)
        {
            int l_actual = evbuffer_remove(l_input, l_buff.data(), l_size);



            if (l_actual == 0)
                break;

            if (l_actual < 0)
            {
                cout << "Failed to drain buffer." << endl;
                break;
            }

            l_buff.UpdateSize(l_actual);

            m_rcvHandler(move(l_buff));

            l_buff.Reset(l_size);
        }

        evbuffer_unlock(l_input);
	}
	catch (...)
	{
	    evbuffer_unlock(l_input);
	    throw;
	}

	auto l_end = high_resolution_clock::now();

	UpdateProcTime(duration_cast<microseconds>(l_end - l_start));
}

inline void CTcpDataConnection::Impl::p_EventCallback(bufferevent* a_evt, short a_flags)
{
	bool l_close = false;

	if (a_flags & BEV_EVENT_CONNECTED)
	{
	    if (not m_error)
	        m_open = true;
	}
	else if (a_flags & BEV_EVENT_ERROR)
	{
		l_close = true;
		m_error = true;
		cout << "Error on connection" << endl;
	}
	else if (a_flags & BEV_EVENT_EOF)
	{
		l_close = true;
		cout << "Client Closed." << endl;
	}

	{
	    // Grab this just to prevent a race condition
	    lock_guard<recursive_mutex> l_lock(m_openLock);
	    m_connecting = false;
	}

	m_openCV.notify_all();

	if (l_close)
    {
        Close();
    }
}

inline void CTcpDataConnection::Impl::UpdateProcTime(const dirus& a_dur)
{
	m_procTime += a_dur.count();
}

inline void CTcpDataConnection::Impl::s_WriteCallback(bufferevent* a_evt, void* a_ptr)
{
    if (a_ptr)
        ((Impl*)a_ptr)->p_WriteCallback(a_evt);
}

inline void CTcpDataConnection::Impl::s_ReadCallback(bufferevent* a_evt, void* a_ptr)
{
    if (a_ptr)
        ((Impl*)a_ptr)->p_ReadCallback(a_evt);
}

inline void CTcpDataConnection::Impl::s_EventCallback(bufferevent* a_evt, short a_flags, void* a_ptr)
{
    if (a_ptr)
        ((Impl*)a_ptr)->p_EventCallback(a_evt, a_flags);
}

}
}
}



#endif /* TCP_DATA_CONNECTION_IMPL_H_ */
