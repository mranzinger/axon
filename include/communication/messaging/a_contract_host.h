/*
 * File description: i_contract_host.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef I_CONTRACT_HOST_H_
#define I_CONTRACT_HOST_H_

#include <unordered_map>
#include <mutex>

#include "contract.h"

namespace axon { namespace communication {

class IContractHandler;

typedef std::shared_ptr<IContractHandler> IContractHandlerPtr;

template<typename ContractType, typename HandlerType>
class CContractHandler;

class AContractHost
{
private:
	typedef std::unordered_map<std::string, IContractHandlerPtr> HandlerMap;

	HandlerMap m_handlers;
	mutable std::mutex m_handlerLock;

public:
	virtual ~AContractHost() = 0;

	template<typename ContractType, typename HandlerType>
	IContractHandlerPtr HostContract(const ContractType &a_contract, const HandlerType &a_handler)
	{
		auto l_handler = std::make_shared<
				CContractHandler<ContractType, HandlerType>
			 >(a_contract, a_handler);

		std::lock_guard<std::mutex> l_lock(m_handlerLock);

		if (!m_handlers.insert(HandlerMap::value_type(a_contract.GetAction(), l_handler)).second)
			throw std::runtime_error("The specified contract action is already being hosted.");

		return l_handler;
	}

	void Adopt(const AContractHost &a_other);

	bool RemoveContract(const std::string &a_action);

protected:
	CMessage::Ptr p_Handle(const CMessage &a_msg) const;

	IContractHandlerPtr p_FindHandler(const std::string &a_action) const;
};

class IContractHandler
{
public:
	virtual ~IContractHandler() { }

	virtual const std::string &GetAction() const = 0;

	virtual CMessage::Ptr Invoke(const CMessage &a_msg) const = 0;
};

template<typename ContractType, typename HandlerType>
class CContractHandler
	: public virtual IContractHandler
{
private:
	ContractType m_contract;
	HandlerType m_handler;

public:
	virtual const std::string &GetAction() const override { return m_contract.GetAction(); }

	virtual CMessage::Ptr Invoke(const CMessage &a_msg) const override
	{
		return m_contract.Invoke(a_msg, m_handler);
	}
};

class CScopedContractHandler
{
private:
	AContractHost &m_host;
	IContractHandlerPtr m_handler;

public:
	template<typename ContractType, typename HandlerType>
	CScopedContractHandler(AContractHost &a_host, const ContractType &a_contract, const HandlerType &a_handler)
		: m_host(a_host)
	{
		m_handler = m_host.HostContract(a_contract, a_handler);
	}

	~CScopedContractHandler()
	{
		m_host.RemoveContract(m_handler->GetAction());
	}
};


} }



#endif /* I_CONTRACT_HOST_H_ */
