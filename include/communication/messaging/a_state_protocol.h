/*
 * File description: a_state_protocol.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef A_STATE_PROTOCOL_H_
#define A_STATE_PROTOCOL_H_

#include <mutex>
#include <queue>

#include "a_protocol.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API AStateProtocol
	: public AProtocol
{
private:
	std::mutex m_queueLock;
	std::mutex m_procLock;
	std::queue<CDataBuffer> m_buffQueue;

public:
	virtual void Process(CDataBuffer a_buffer);

protected:
	virtual void ProcessInternal(CDataBuffer a_buffer) = 0;

private:
	void TryProcess(bool a_tryOnExit);
};

} }



#endif /* A_STATE_PROTOCOL_H_ */
