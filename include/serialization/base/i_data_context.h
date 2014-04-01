/*
 * File description: i_data_context.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef I_DATA_CONTEXT_H_
#define I_DATA_CONTEXT_H_

#include <memory>

#include "../dll_export.h"

namespace axon { namespace serialization {

class AXON_SERIALIZE_API IDataContext
{
public:
	typedef std::unique_ptr<IDataContext> Ptr;

	virtual ~IDataContext() { }
};

} }



#endif /* I_DATA_CONTEXT_H_ */
