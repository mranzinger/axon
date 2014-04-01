/*
 * File description: i_protocol.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef I_PROTOCOL_H_
#define I_PROTOCOL_H_

#include <functional>

#include "message.h"
#include "data_buffer.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API IProtocol
{
public:
	typedef std::unique_ptr<IProtocol> Ptr;

	typedef std::function<void (CMessage::Ptr)> HandlerFn;

	virtual ~IProtocol() { }

	virtual CDataBuffer SerializeMessage(const CMessage &a_msg) const = 0;

	virtual void Process(CDataBuffer a_buffer) = 0;

	virtual void SetHandler(HandlerFn a_fn) = 0;
};

AXON_COMMUNICATE_API IProtocol::Ptr GetDefaultProtocol();

} }

#endif /* I_PROTOCOL_H_ */
