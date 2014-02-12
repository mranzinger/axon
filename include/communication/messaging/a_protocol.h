/*
 * File description: a_protocol.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef A_PROTOCOL_H_
#define A_PROTOCOL_H_

#include "i_protocol.h"

namespace axon { namespace communication {

class AProtocol
	: public virtual IProtocol
{
private:
	HandlerFn m_handler;

public:
	virtual void SetHandler(HandlerFn a_fn) override
	{
		m_handler = std::move(a_fn);
	}

protected:
	void MessageProcessed(const CMessage::Ptr &a_msg)
	{
		m_handler(a_msg);
	}
};

} }



#endif /* A_PROTOCOL_H_ */
