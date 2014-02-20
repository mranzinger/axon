/*
 * fault_exception.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef FAULT_EXCEPTION_H_
#define FAULT_EXCEPTION_H_

#include <stdexcept>

#include "dll_export.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API CFaultException
	: public std::runtime_error
{
private:
	int m_errorCode;

public:
	CFaultException()
		: std::runtime_error(""), m_errorCode(-1) { }
	CFaultException(const std::string &a_what)
		: std::runtime_error(a_what), m_errorCode(-1)
	{
	}
	CFaultException(const std::string &a_what, int errorCode)
		: std::runtime_error(a_what), m_errorCode(errorCode)
	{
	}

	int ErrorCode() const { return m_errorCode; }

	int &ErrorCode() { return m_errorCode; }
};

} }



#endif /* FAULT_EXCEPTION_H_ */
