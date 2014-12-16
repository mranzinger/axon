/*
 * File description: a_contract_host.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */


#include "communication/messaging/a_contract_host.h"
#include "communication/fault_exception.h"

using namespace std;

namespace axon { namespace communication {

AContractHost::AContractHost()
{
    HostContract(QUERY_CONTRACTS_CONTRACT,
                 bind(&AContractHost::p_QueryContracts, this));
}

set<string> AContractHost::p_QueryContracts() const
{
    return QueryContracts();
}

set<string> AContractHost::QueryContracts() const
{
    set<string> l_ret;

    {
        lock_guard<mutex> l_lock(m_handlerLock);

        for (const pair<string, IContractHandlerPtr> &l_p : m_handlers)
        {
            l_ret.insert(l_p.first);
        }
    }

    return move(l_ret);
}

void AContractHost::Host(IContractHandlerPtr a_handler)
{
	lock_guard<mutex> l_lock(m_handlerLock);

	if (!m_handlers.insert(HandlerMap::value_type(a_handler->GetAction(), a_handler)).second)
		throw runtime_error("The specified contract action is already being hosted.");
}

CMessage::Ptr AContractHost::Handle(const CMessage& a_msg) const
{
	CMessage::Ptr l_ret;
	if (!TryHandle(a_msg, l_ret))
		throw runtime_error("Unable to locate message handler for action '" + a_msg.GetAction() + "'.");

	return move(l_ret);
}

bool AContractHost::TryHandle(const CMessage& a_msg, CMessage::Ptr& a_out) const
{
	try
	{
		IContractHandlerPtr l_handler = FindHandler(a_msg.GetAction());

		if (!l_handler)
			return false;

		a_out = l_handler->Invoke(a_msg);
	}
	catch (CFaultException &ex)
	{
		a_out = make_shared<CMessage>(a_msg, ex);
	}
	catch (exception &ex)
	{
		a_out = make_shared<CMessage>(a_msg, ex);
	}

	return true;
}

bool AContractHost::RemoveContract(const std::string& a_action)
{
	lock_guard<mutex> l_lcok(m_handlerLock);

	return m_handlers.erase(a_action) != 0;
}



IContractHandlerPtr AContractHost::FindHandler(const std::string& a_action) const
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


