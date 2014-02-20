/*
 * fault_serialization.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef FAULT_SERIALIZATION_H_
#define FAULT_SERIALIZATION_H_

#include "serialization/master.h"
#include "../fault_exception.h"

namespace axon { namespace serialization {

AXON_COMMUNICATE_API void WriteStruct(const CStructWriter &a_writer, const communication::CFaultException &a_ex);
AXON_COMMUNICATE_API void ReadStruct(const CStructReader &a_reader, communication::CFaultException &a_ex);

} }

#endif /* FAULT_SERIALIZATION_H_ */
