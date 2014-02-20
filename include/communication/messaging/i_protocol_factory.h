/*
 * File description: i_protocol_factory.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef I_PROTOCOL_FACTORY_H_
#define I_PROTOCOL_FACTORY_H_

#include <tuple>

#include "i_protocol.h"
#include "function_invoker.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API IProtocolFactory
{
public:
	typedef std::shared_ptr<IProtocolFactory> Ptr;

	virtual ~IProtocolFactory() { }

	virtual IProtocol::Ptr Create() const = 0;
};

template<typename Protocol, typename ...Args>
class CProtocolFactory
	: public IProtocolFactory
{
private:
	std::tuple<Args...> m_args;
	typename internal::CGenSequence<sizeof...(Args)>::type
			m_sequence;

public:
	CProtocolFactory(Args &&...a_args)
		: m_args(std::forward<Args>(a_args)...)
	{
	}

	virtual IProtocol::Ptr Create() const override
	{
		return p_Create(m_sequence);
	}

private:
	template<int ...Seq>
	IProtocol::Ptr p_Create(const internal::CSequence<Seq...> &) const
	{
		return IProtocol::Ptr(new Protocol(std::get<Seq>(m_args)...));
	}
};

template<typename Protocol, typename ...Args>
IProtocolFactory::Ptr GetProtocolFactory(Args &&...a_args)
{
	return std::make_shared<CProtocolFactory<Protocol, Args...>>(std::forward<Args>(a_args)...);
}

AXON_COMMUNICATE_API IProtocolFactory::Ptr GetDefaultProtocolFactory();

} }

#endif /* I_PROTOCOL_FACTORY_H_ */
