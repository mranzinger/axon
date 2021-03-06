/*
 * function_invoker.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef FUNCTION_INVOKER_H_
#define FUNCTION_INVOKER_H_

#include <tuple>
#include <functional>

#include "util/return_type.h"

namespace axon { namespace communication {

namespace internal {

template<int ...Sequence>
struct CSequence { };

template<int Val, int ...Sequence>
struct CGenSequence
	: CGenSequence<Val -1, Val -1, Sequence...> { };

template<int ...Sequence>
struct CGenSequence<0, Sequence...>
{
	typedef CSequence<Sequence...> type;
};

template<typename Fn, typename Tuple, int ...Seq>
typename util::return_type<Fn>::type invoke_fn(Fn &&a_fn, Tuple a_tuple, const CSequence<Seq...> &)
{
	return a_fn(std::move(std::get<Seq>(a_tuple))...);
}

}

template<typename Fn, typename ...Args>
typename util::return_type<Fn>::type InvokeFunction(Fn &&a_fn, std::tuple<Args...> a_tuple)
{
	return internal::invoke_fn(std::forward<Fn>(a_fn), std::move(a_tuple),
			typename internal::CGenSequence<sizeof...(Args)>::type());
}

//template<typename Ret, typename ...Args>
//Ret InvokeFunction(const std::function<Ret (Args...)> &a_fn, const std::tuple<Args...> &a_tuple)
//{
//	return invoke_fn(a_fn, a_tuple, typename CGenSequence<sizeof...(Args)>::type());
//}


} }



#endif /* FUNCTION_INVOKER_H_ */
