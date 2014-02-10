/*
 * return_type.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef RETURN_TYPE_H_
#define RETURN_TYPE_H_

#include <type_traits>

namespace axon { namespace util {

namespace {

template<typename F, typename Ret, typename ...Args>
Ret rt_helper(Ret (F::*)(Args...));

template<typename F, typename Ret, typename ...Args>
Ret rt_helper(Ret (F::*)(Args...) const);

}

template<typename F>
struct return_type
{
	typedef decltype(rt_helper(&F::operator())) type;
};

} }



#endif /* RETURN_TYPE_H_ */
