/*
 * File description: a_state_protocol.cpp
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#include "communication/messaging/a_state_protocol.h"

using namespace std;

namespace axon { namespace communication {

void AStateProtocol::Process(CDataBuffer a_buffer)
{
	{
		//lock_guard<mutex> l_lock(m_queueLock);
		m_buffQueue.push(move(a_buffer));
	}

	TryProcess(true);
}

void AStateProtocol::TryProcess(bool a_tryOnExit)
{
	{
		//unique_lock<mutex> l_procLock(m_procLock, try_to_lock);

		//if (!l_procLock.owns_lock())
		//	return;

		while (true)
		{
			CDataBuffer l_nextBuff;

			{
				//lock_guard<mutex> l_qLock(m_queueLock);
				if (m_buffQueue.empty())
					break;
				l_nextBuff = move(m_buffQueue.front());
				m_buffQueue.pop();
			}

			ProcessInternal(move(l_nextBuff));
		}
	}

	// There is a weird race condition that can occur due to the
	// lazy processing mechanism. Trying to process again after relinquishing
	// the lock to the process fixes the race
	if (a_tryOnExit)
		TryProcess(false);
}

}
}


