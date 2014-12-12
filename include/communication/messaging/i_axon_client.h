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

class IMessageWaitHandle
{
public:
    typedef std::unique_ptr<IMessageWaitHandle> Ptr;

    virtual ~IMessageWaitHandle() { }

    virtual void Wait() = 0;
    virtual CMessage::Ptr GetMessage() = 0;
};

template<typename Ret>
class IContractWaitHandle
{
public:
    typedef Ret return_type;
    typedef std::unique_ptr<IContractWaitHandle> Ptr;

    virtual ~IContractWaitHandle() { }

    virtual void Wait() = 0;
    virtual return_type Get() = 0;
};

template<typename ContractType>
class CContractWaitHandle
    : public IContractWaitHandle<typename ContractType::return_type>
{
public:
    typedef typename ContractType::return_type return_type;
    typedef std::unique_ptr<CContractWaitHandle> Ptr;

    CContractWaitHandle(const ContractType &a_contract,
                        IMessageWaitHandle::Ptr a_waitHandle);

    virtual void Wait() override;

    virtual return_type Get() override;

private:
    const ContractType &m_contract;
    bool m_valid;
    return_type m_ret;
    IMessageWaitHandle::Ptr m_waitHandle;
};

class AXON_COMMUNICATE_API IAxonClient
	: public virtual IContractHost
{
public:
	typedef std::shared_ptr<IAxonClient> Ptr;

	virtual ~IAxonClient() { }

	virtual void Connect(const std::string &a_connectionString) = 0;
	virtual void Connect(IDataConnection::Ptr a_connection) = 0;
	virtual void Close() = 0;

	virtual bool IsOpen() const = 0;

	virtual void SetProtocol(IProtocol::Ptr a_protocol) = 0;

    virtual std::string ConnectionString() const = 0;

	virtual CMessage::Ptr Send(const CMessage::Ptr &a_message) = 0;
	virtual CMessage::Ptr Send(const CMessage::Ptr &a_message, uint32_t a_timeout) = 0;
	virtual IMessageWaitHandle::Ptr SendAsync(const CMessage::Ptr &a_message) = 0;
	virtual IMessageWaitHandle::Ptr SendAsync(const CMessage::Ptr &a_message, uint32_t a_timeout) = 0;
	virtual void SendNonBlocking(const CMessage::Ptr &a_message) = 0;

	template<typename Ret, typename ...Args>
	Ret Send(const CContract<Ret (Args...)> &a_contract, const Args &...a_args)
	{
		return Send(a_contract, 0, a_args...);
	}

	template<typename Ret, typename ...Args>
	Ret Send(const CContract<Ret (Args...)> &a_contract, uint32_t a_timeout, const Args &...a_args)
	{
		CMessage::Ptr l_send = a_contract.Serialize(a_args...);

		CMessage::Ptr l_ret = Send(l_send, a_timeout);

		Ret l_retval;
		a_contract.DeserializeRet(*l_ret, l_retval);

		return std::move(l_retval);
	}

	template<typename Ret, typename ...Args>
	typename IContractWaitHandle<Ret>::Ptr
	    SendAsync(const CContract<Ret (Args...)> &a_contract, const Args &...a_args)
	{
	    return SendAsync(a_contract, 0, a_args...);
	}

	template<typename Ret, typename ...Args>
    typename IContractWaitHandle<Ret>::Ptr
        SendAsync(const CContract<Ret (Args...)> &a_contract, uint32_t a_timeout, const Args &...a_args)
    {
	    typedef CContractWaitHandle<CContract<Ret (Args...)>> handle_type;

        CMessage::Ptr l_send = a_contract.Serialize(a_args...);

        IMessageWaitHandle::Ptr l_waitHandle = SendAsync(l_send, a_timeout);

        typename handle_type::Ptr l_ret(new handle_type(a_contract, std::move(l_waitHandle)));

        return std::move(l_ret);
    }

	template<typename ...Args>
	void VSend(const CContract<void (Args...)> &a_contract, const Args &...a_args)
	{
		VSend(a_contract, 0, a_args...);
	}

	template<typename ...Args>
	void VSend(const CContract<void (Args...)> &a_contract, uint32_t a_timeout, const Args &...a_args)
	{
		CMessage::Ptr l_send = a_contract.Serialize(a_args...);

		(void) Send(l_send, a_timeout);
	}

    static IAxonClient *GetExecutingInstance();

protected:
    static void SetExecutingInstance(IAxonClient *a_instance);
};

template<typename ContractType>
CContractWaitHandle<ContractType>::CContractWaitHandle(
        const ContractType &a_contract,
        IMessageWaitHandle::Ptr a_waitHandle)
    : m_contract(a_contract), m_waitHandle(std::move(a_waitHandle)), m_valid(false)
{
}

template<typename ContractType>
void CContractWaitHandle<ContractType>::Wait()
{
    if (m_valid)
        return;

    m_waitHandle->Wait();
    m_valid = true;
}

template<typename ContractType>
typename CContractWaitHandle<ContractType>::return_type CContractWaitHandle<ContractType>::Get()
{
    Wait();

    CMessage::Ptr l_message = m_waitHandle->GetMessage();

    m_contract.DeserializeRet(*l_message, m_ret);

    return m_ret;
}

}
}



#endif /* I_AXON_CLIENT_H_ */
