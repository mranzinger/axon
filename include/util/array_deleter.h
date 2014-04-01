/*
 * File description: array_deleter.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
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
