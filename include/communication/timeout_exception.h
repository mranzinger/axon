/*
 * File description: timeout_exception.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef TIMEOUT_EXCEPTION_H_
#define TIMEOUT_EXCEPTION_H_

#include <stdexcept>

#include "dll_export.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API CTimeoutException
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
