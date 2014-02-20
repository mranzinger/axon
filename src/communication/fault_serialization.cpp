/*
 * fault_serialization.cpp
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#include "communication/messaging/fault_serialization.h"

namespace axon { namespace serialization {

void WriteStruct(const CStructWriter& a_writer, const communication::CFaultException& a_ex)
{
	a_writer("What", a_ex.what())
			("Code", a_ex.ErrorCode());
}

void ReadStruct(const CStructReader& a_reader, communication::CFaultException& a_ex)
{
	std::string what;
	int code;

	a_reader("What", what)
			("Code", code);

	a_ex = communication::CFaultException(std::move(what), code);
}

}
}


