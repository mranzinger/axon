/*
 * File description: axon_client.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
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
	: m_connection(move(a_connection))
{
	SetDefaultProtocol();
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
		m_connection->SetReceiveHandler(bind(&CAxonClient::p_OnDataReceived, this, placeholders::_1, placeholders::_2));
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

CMessage::Ptr CAxonClient::Send(const CMessage &a_message, uint32_t a_timeout)
{
	if (!m_connection || !m_connection->IsOpen())
		throw runtime_error("Cannot send data over a dead connection.");

	p_SendNonBlocking(a_message);

	auto l_start = chrono::steady_clock::now();

	auto l_durLeft = chrono::milliseconds(a_timeout);

	CMessageSocket l_socket(a_message);

	unique_lock<mutex> l_waitLock(m_pendingLock);

	// Add the socket to the map of messages waiting to be handled
	m_pendingList.push_back(&l_socket);

	size_t l_sockIdx = m_pendingList.size() - 1;

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
			m_pendingList.erase(m_pendingList.begin() + l_sockIdx);

			if (!l_socket.IncomingMessage)
			{
				throw CTimeoutException(a_timeout);
			}
		}
	}

	return move(l_socket.IncomingMessage);
}

void CAxonClient::p_SendNonBlocking(const CMessage& a_message)
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
		p_SendNonBlocking(*l_response);
		l_handled = true;
	}

	if (l_handled)
		return;

	// TODO: Allow the server to handle this
}

void CAxonClient::p_OnDataReceived(char* a_buffer, int a_bufferSize)
{
	// This function is invoked whenever the data connection
	// signals that data has been received from the remote peer.
	// In the case of the AxonClient, this should be immediately forwarded to
	// the protocol so that a message can be reconstructed from it.
	// The data connection owns the buffer, so a copy must be made

	m_protocol->Process(CDataBuffer::Copy(a_buffer, a_bufferSize));
}

}
}


