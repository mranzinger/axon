/*
 * File description: i_axon_client.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef I_AXON_CLIENT_H_
#define I_AXON_CLIENT_H_

#include "i_protocol.h"
#include "../i_data_connection.h"
#include "i_contract_host.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API IAxonClient
	: public virtual IContractHost
{
public:
	typedef std::shared_ptr<IAxonClient> Ptr;

	virtual ~IAxonClient() { }

	virtual void Connect(const std::string &a_connectionString) = 0;
	virtual void Connect(IDataConnection::Ptr a_connection) = 0;

	virtual void SetProtocol(IProtocol::Ptr a_protocol) = 0;

	virtual CMessage::Ptr Send(const CMessage &a_message) = 0;
	virtual CMessage::Ptr Send(const CMessage &a_message, uint32_t a_timeout) = 0;
	virtual void SendNonBlocking(const CMessage &a_message) = 0;

	template<typename Ret, typename ...Args>
	Ret Send(const CContract<Ret (Args...)> &a_contract, const Args &...a_args)
	{
		return Send(a_contract, 0, a_args...);
	}

	template<typename Ret, typename ...Args>
	Ret Send(const CContract<Ret (Args...)> &a_contract, uint32_t a_timeout, const Args &...a_args)
	{
		CMessage::Ptr l_send = a_contract.Serialize(a_args...);

		CMessage::Ptr l_ret = Send(*l_send, a_timeout);

		Ret l_retval;
		a_contract.DeserializeRet(*l_ret, l_retval);

		return std::move(l_retval);
	}

	template<typename ...Args>
	void Send(const CContract<void (Args...)> &a_contract, const Args &...a_args)
	{
		Send(a_contract, 0, a_args...);
	}

	template<typename ...Args>
	void Send(const CContract<void (Args...)> &a_contract, uint32_t a_timeout, const Args &...a_args)
	{
		CMessage::Ptr l_send = a_contract.Serialize(a_args...);

		(void) Send(*l_send, a_timeout);
	}
};

} }



#endif /* I_AXON_CLIENT_H_ */
