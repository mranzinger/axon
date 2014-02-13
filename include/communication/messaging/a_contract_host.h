/*
 * File description: i_contract_host.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef A_CONTRACT_HOST_H_
#define A_CONTRACT_HOST_H_

#include <unordered_map>
#include <mutex>

#include "i_contract_host.h"

namespace axon { namespace communication {



class AContractHost
	: public virtual IContractHost
{
private:
	typedef std::unordered_map<std::string, IContractHandlerPtr> HandlerMap;

	HandlerMap m_handlers;
	mutable std::mutex m_handlerLock;

public:
	virtual void HostContract(IContractHandlerPtr a_handler) override;

	void Adopt(const AContractHost &a_other);

	virtual bool RemoveContract(const std::string &a_action) override;

	virtual CMessage::Ptr Handle(const CMessage &a_msg) const override;
	virtual bool TryHandle(const CMessage &a_msg, CMessage::Ptr &a_out) const override;

	virtual IContractHandlerPtr FindHandler(const std::string &a_action) const override;

protected:
	AContractHost() { }
};




} }



#endif /* A_CONTRACT_HOST_H_ */
