/*
 * File description: i_data_context.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef I_DATA_CONTEXT_H_
#define I_DATA_CONTEXT_H_

#include <memory>

namespace axon { namespace serialization {

class IDataContext
{
public:
	typedef std::unique_ptr<IDataContext> Ptr;

	virtual ~IDataContext() { }
};

} }



#endif /* I_DATA_CONTEXT_H_ */
