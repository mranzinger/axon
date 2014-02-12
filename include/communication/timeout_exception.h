/*
 * File description: timeout_exception.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef TIMEOUT_EXCEPTION_H_
#define TIMEOUT_EXCEPTION_H_

#include <stdexcept>

namespace axon { namespace communication {

class CTimeoutException
	: public std::runtime_error
{
private:
	uint32_t m_timeout;

public:
	CTimeoutException(uint32_t a_timeout)
		: std::runtime_error("A timeout occurred."), m_timeout(a_timeout)
	{

	}

	uint32_t Timeout() const { return m_timeout; }
};

} }



#endif /* TIMEOUT_EXCEPTION_H_ */
