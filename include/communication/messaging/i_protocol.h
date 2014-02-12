/*
 * File description: i_protocol.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef I_PROTOCOL_H_
#define I_PROTOCOL_H_

#include <functional>

#include "message.h"
#include "data_buffer.h"

namespace axon { namespace communication {

class IProtocol
{
public:
	typedef std::shared_ptr<IProtocol> Ptr;

	typedef std::function<void (CMessage::Ptr)> HandlerFn;

	virtual ~IProtocol() { }

	virtual CDataBuffer SerializeMessage(const CMessage &a_msg) const = 0;

	virtual void Process(CDataBuffer a_buffer) = 0;

	virtual void SetHandler(HandlerFn a_fn) = 0;
};

} }

#endif /* I_PROTOCOL_H_ */
