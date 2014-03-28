/*
 * File description: axon_client.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include "communication/messaging/axon_client.h"
#include "communication/timeout_exception.h"

#include <functional>

using namespace std;

namespace axon { namespace communication {

struct CMessageSocket
{
	CMessageSocket(const CMessage &a_outboundMessage)
		: OutboundMessage(a_outboundMessage)
	{
	}

	const CMessage &OutboundMessage;
	CMessage::Ptr IncomingMessage;
};

CAxonClient::CAxonClient()
{
	SetDefaultProtocol();
}

CAxonClient::CAxonClient(const std::string& a_connectionString)
{
	SetDefaultProtocol();

	Connect(a_connectionString);
}

CAxonClient::CAxonClient(IDataConnection::Ptr a_connection)
{
	SetDefaultProtocol();

	Connect(move(a_connection));
}

CAxonClient::CAxonClient(const std::string& a_connectionString, IProtocol::Ptr a_protocol)
{
	SetProtocol(move(a_protocol));

	Connect(a_connectionString);
}

CAxonClient::CAxonClient(IDataConnection::Ptr a_connection, IProtocol::Ptr a_protocol)
{
	SetProtocol(move(a_protocol));

	Connect(move(a_connection));
}





CAxonClient::Ptr CAxonClient::Create()
{
	return make_shared<CAxonClient>();
}

CAxonClient::Ptr CAxonClient::Create(const std::string& a_connectionString)
{
	return make_shared<CAxonClient>(a_connectionString);
}

CAxonClient::Ptr CAxonClient::Create(IDataConnection::Ptr a_connection)
{
	return make_shared<CAxonClient>(move(a_connection));
}

CAxonClient::Ptr CAxonClient::Create(const std::string& a_connectionString,
		IProtocol::Ptr a_protocol)
{
	return make_shared<CAxonClient>(a_connectionString, move(a_protocol));
}

CAxonClient::Ptr CAxonClient::Create(IDataConnection::Ptr a_connection,
		IProtocol::Ptr a_protocol)
{
	return make_shared<CAxonClient>(move(a_connection), move(a_protocol));
}

void CAxonClient::Connect(const std::string& a_connectionString)
{
	Connect(IDataConnection::Create(a_connectionString));
}

void CAxonClient::Connect(IDataConnection::Ptr a_connection)
{
	m_connection = move(a_connection);

	if (m_connection)
	{
#ifdef IS_WINDOWS
		int l_hack;
		m_connection->SetReceiveHandler(
			[this, l_hack](CDataBuffer a_buf)
			{
				p_OnDataReceived(move(a_buf));
			});
#else
		m_connection->SetReceiveHandler(bind(&CAxonClient::p_OnDataReceived, this, placeholders::_1));
#endif
	}
}

void CAxonClient::SetDefaultProtocol()
{
	SetProtocol(GetDefaultProtocol());
}

void CAxonClient::SetProtocol(IProtocol::Ptr a_protocol)
{
	if (!a_protocol)
		throw runtime_error("Invalid protocol. Cannot be null.");

	m_protocol = move(a_protocol);

	m_protocol->SetHandler(bind(&CAxonClient::p_OnMessageReceived, this, placeholders::_1));
}

string CAxonClient::ConnectionString() const
{
    if (!m_connection)
        return "";
    return m_connection->ConnectionString();
}

CMessage::Ptr CAxonClient::Send(const CMessage& a_message)
{

	return Send(a_message, 0);
}

CMessage::Ptr CAxonClient::Send(const CMessage &a_message, uint32_t a_timeout)
{
	if (!m_connection || !m_connection->IsOpen())
		throw runtime_error("Cannot send data over a dead connection.");
	// Default timeout is 1 minute
	if (a_timeout == 0)
		a_timeout = 60000;

	p_Send(a_message);

	auto l_start = chrono::steady_clock::now();

	auto l_durLeft = chrono::milliseconds(a_timeout);

	CMessageSocket l_socket(a_message);

	unique_lock<mutex> l_waitLock(m_pendingLock);

	// Add the socket to the map of messages waiting to be handled
	m_pendingList.push_back(&l_socket);

	while (!l_socket.IncomingMessage)
	{
		cv_status l_status = cv_status::no_timeout;

		if (a_timeout)
			l_status = m_newMessageEvent.wait_for(l_waitLock, l_durLeft);
		else
			m_newMessageEvent.wait(l_waitLock);

		l_durLeft -= chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - l_start);

		if (l_socket.IncomingMessage
				|| l_status == cv_status::timeout
				|| l_durLeft < chrono::milliseconds(0))
		{
			auto iter = find(m_pendingList.begin(), m_pendingList.end(), &l_socket);
            
            if (iter != m_pendingList.end())
                m_pendingList.erase(iter);
            else
                throw runtime_error("The completed socket wasn't in the list of pending communications."
                                    " Not even sure this can happen... but if it did... bummer.");

			if (!l_socket.IncomingMessage)
			{
				throw CTimeoutException(a_timeout);
			}
		}
	}

	return move(l_socket.IncomingMessage);
}

void CAxonClient::SendNonBlocking(const CMessage& a_message)
{
	// Hacky, but we need to flag the message as one way so that
	// a response doesn't get generated
	const_cast<CMessage&>(a_message).SetOneWay(true);

	p_Send(a_message);
}

void CAxonClient::p_Send(const CMessage& a_message)
{
	CDataBuffer l_buffer = m_protocol->SerializeMessage(a_message);

	m_connection->Send(l_buffer.ToShared());
}

void CAxonClient::p_OnMessageReceived(const CMessage::Ptr& a_message)
{
	// This function is invoked whenever the protocol has reconstructed
	// a message. There are 3 things that this object needs to try before
	// erroring out:
	// 1) Check to see if this message is a response from an outbound call
	//    Resolution: Add the message to the new messages map and signal
	//                that there is a new message
	// 2) Check to see if this instance has a handler for this message
	//    Resolution: Handle the message and send the return value back out
	// 3) If this client is a child of a server, then see if the server
	//      can handle the message.
	//
	// If none of the steps succeed, then throw a fault

    SetExecutingInstance(this);

	bool l_handled = false;

	{
		lock_guard<mutex> l_lock(m_pendingLock);

		const std::string &l_reqId = a_message->RequestId();

		// See if the RequestId of this message is the Id of a message
		// in the outbound list
		auto iter = find_if(m_pendingList.begin(), m_pendingList.end(),
				[&l_reqId] (CMessageSocket *l_sock)
				{
					return l_reqId == l_sock->OutboundMessage.Id();
				});

		// This message is a result of an outbound request, so let
		// the blocking outbound requests know
		if (iter != m_pendingList.end())
		{
			(*iter)->IncomingMessage = a_message;
			l_handled = true;
		}
	}

	if (l_handled)
	{
		// Wake up everyone that is currently waiting
		m_newMessageEvent.notify_all();
		return;
	}

	// Ok, so this message isn't a result of an outbound call, so see if this
	// client has a handler for it
	CMessage::Ptr l_response;
	if (TryHandle(*a_message, l_response))
	{
		// There was a handler for this message, so now
		// send it back out to the caller
		if (!a_message->IsOneWay())
			SendNonBlocking(*l_response);
		l_handled = true;
	}

	if (l_handled)
		return;

	if (TryHandleWithServer(*a_message, l_response))
	{
		if (!a_message->IsOneWay())
			SendNonBlocking(*l_response);
		l_handled = true;
	}

	if (l_handled)
		return;

	// If this message is not a request, then send a fault back to the caller
	if (a_message->RequestId().empty() && !a_message->IsOneWay())
	{
		l_response = make_shared<CMessage>(*a_message,
				CFaultException("The action '" + a_message->GetAction() + "' has no supported handlers."));
		SendNonBlocking(*l_response);
	}
	else
	{
		// This is probably due to a timeout
		// TODO: Log this
	}
}

void CAxonClient::p_OnDataReceived(CDataBuffer a_buffer)
{
	// This function is invoked whenever the data connection
	// signals that data has been received from the remote peer.
	// In the case of the AxonClient, this should be immediately forwarded to
	// the protocol so that a message can be reconstructed from it.
	// The data connection owns the buffer, so a copy must be made

	m_protocol->Process(move(a_buffer));
}

bool CAxonClient::TryHandleWithServer(const CMessage& a_msg, CMessage::Ptr& a_out) const
{
	// Derived instances need to override this
	return false;
}

}
}


