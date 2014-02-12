/*
 * File description: axon_client.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#include "communication/messaging/axon_client.h"
#include "communication/timeout_exception.h"

using namespace std;

namespace axon { namespace communication {

CAxonClient::CAxonClient()
{
	SetDefaultProtocol();
}

CAxonClient::CAxonClient(const std::string& a_connectionString)
	: CAxonClient()
{
	Connect(a_connectionString);
}

CAxonClient::CAxonClient(IDataConnection::Ptr a_connection)
	: CAxonClient(), m_connection(move(a_connection))
{
}

void CAxonClient::Connect(const std::string& a_connectionString)
{
	m_connection = IDataConnection::Create(a_connectionString);
}

void CAxonClient::Connect(IDataConnection::Ptr a_connection)
{
	m_connection = move(a_connection);
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
}

CMessage::Ptr CAxonClient::Send(const CMessage &a_message, uint32_t a_timeout)
{
	CDataBuffer l_buffer = m_protocol->SerializeMessage(a_message);

	auto l_start = chrono::steady_clock::now();

	auto l_durLeft = chrono::milliseconds(a_timeout);

	CMessage::Ptr l_ret;

	unique_lock<mutex> l_waitLock(m_newMessageLock);

	while (!l_ret)
	{
		cv_status l_status = cv_status::no_timeout;

		if (a_timeout)
			l_status = m_newMessageEvent.wait_for(l_waitLock, l_durLeft);
		else
			m_newMessageEvent.wait(l_waitLock);

		if (l_status == cv_status::timeout)
			throw CTimeoutException(a_timeout);

		auto iter = m_newMessages.find(a_message.Id());

		if (iter != m_newMessages.end())
		{
			l_ret = move(iter->second);
			m_newMessages.erase(iter);
		}
		else
		{
			l_durLeft -= (chrono::steady_clock::now() - l_start);

			if (l_durLeft < 0)
				throw CTimeoutException(a_timeout);
		}
	}

	return l_ret;
}

}
}


