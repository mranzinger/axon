/*
 * File description: i_contract_host.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef A_CONTRACT_HOST_H_
#define A_CONTRACT_HOST_H_

#include <unordered_map>
#include <mutex>

#include "i_contract_host.h"

namespace axon { namespace communication {



class AXON_COMMUNICATE_API AContractHost
	: public virtual IContractHost
{
private:
	typedef std::unordered_map<std::string, IContractHandlerPtr> HandlerMap;

	HandlerMap m_handlers;
	mutable std::mutex m_handlerLock;

public:
	virtual void Host(IContractHandlerPtr a_handler) override;

	void Adopt(const AContractHost &a_other);

	virtual bool RemoveContract(const std::string &a_action) override;

	virtual CMessage::Ptr Handle(const CMessage &a_msg) const override;
	virtual bool TryHandle(const CMessage &a_msg, CMessage::Ptr &a_out) const override;

	virtual IContractHandlerPtr FindHandler(const std::string &a_action) const override;

	virtual std::vector<std::string> QueryContracts() const override;

protected:
	AContractHost();

	std::vector<std::string> p_QueryContracts() const;
};




} }



#endif /* A_CONTRACT_HOST_H_ */
