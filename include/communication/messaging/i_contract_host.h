/*
 * File description: i_contract_host.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef I_CONTRACT_HOST_H_
#define I_CONTRACT_HOST_H_

#include "contract.h"

namespace axon { namespace communication {

class IContractHandler;
typedef std::shared_ptr<IContractHandler> IContractHandlerPtr;

template<typename ContractType, typename HandlerType>
class CContractHandler;

class AXON_COMMUNICATE_API IContractHost
{
public:
	typedef std::shared_ptr<IContractHost> Ptr;

	virtual ~IContractHost() { }

	virtual void Host(IContractHandlerPtr a_handler) = 0;

	template<typename ContractType, typename HandlerType>
	IContractHandlerPtr HostContract(ContractType a_contract, HandlerType a_handler)
	{
		auto l_handler = std::make_shared<
				CContractHandler<ContractType, HandlerType>
			 >(std::move(a_contract), std::move(a_handler));

		Host(l_handler);

		return l_handler;
	}

	virtual bool RemoveContract(const std::string &a_action) = 0;

	virtual CMessage::Ptr Handle(const CMessage &a_msg) const = 0;
	virtual bool TryHandle(const CMessage &a_msg, CMessage::Ptr &a_out) const = 0;

	virtual IContractHandlerPtr FindHandler(const std::string &a_action) const = 0;
};

class AXON_COMMUNICATE_API IContractHandler
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
	CContractHandler(ContractType a_contract, HandlerType a_handler)
		: m_contract(std::move(a_contract)), m_handler(std::move(a_handler))
	{
	}

	virtual const std::string &GetAction() const override { return m_contract.GetAction(); }

	virtual CMessage::Ptr Invoke(const CMessage &a_msg) const override
	{
		return m_contract.Invoke(a_msg, m_handler);
	}
};

class AXON_COMMUNICATE_API CScopedContractHandler
{
private:
	IContractHost &m_host;
	IContractHandlerPtr m_handler;

public:
	template<typename ContractType, typename HandlerType>
	CScopedContractHandler(IContractHost &a_host, const ContractType &a_contract, const HandlerType &a_handler)
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
