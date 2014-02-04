/*
 * File description: array_deleter.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef ARRAY_DELETER_H_
#define ARRAY_DELETER_H_

namespace axon { namespace util {

template<typename T>
struct CArrayDeleter
{
	void operator()(const T *p) const
	{
		delete[] p;
	}
};

} }



#endif /* ARRAY_DELETER_H_ */
