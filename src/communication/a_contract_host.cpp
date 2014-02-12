/*
 * File description: a_contract_host.cpp
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */


#include "communication/messaging/a_contract_host.h"
#include "communication/fault_exception.h"

using namespace std;

namespace axon { namespace communication {

CMessage::Ptr AContractHost::p_Handle(const CMessage& a_msg) const
{
	try
	{
		IContractHandlerPtr l_handler = p_FindHandler(a_msg.GetAction());

		if (!l_handler)
			throw runtime_error("Unable to locate message handler for action '" + a_msg.GetAction() + "'.");

		return l_handler->Invoke(a_msg);
	}
	catch (CFaultException &ex)
	{
		return make_shared<CMessage>(a_msg, ex);
	}
	catch (exception &ex)
	{
		return make_shared<CMessage>(a_msg, ex);
	}
}

bool AContractHost::RemoveContract(const std::string& a_action)
{
	lock_guard<mutex> l_lcok(m_handlerLock);

	return m_handlers.erase(a_action) != 0;
}

IContractHandlerPtr AContractHost::p_FindHandler(const std::string& a_action) const
{
	lock_guard<mutex> l_lock(m_handlerLock);

	auto iter = m_handlers.find(a_action);

	if (iter == m_handlers.end())
		return IContractHandlerPtr();
	else
		return iter->second;
}

void AContractHost::Adopt(const AContractHost& a_other)
{
	lock_guard<mutex> l_lock(m_handlerLock);
	lock_guard<mutex> l_lock2(a_other.m_handlerLock);

	m_handlers.insert(a_other.m_handlers.begin(), a_other.m_handlers.end());
}

}
}


