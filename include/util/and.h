/*
 * File description: and.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef AND_H_
#define AND_H_

namespace axon { namespace util {

template<bool ...Args>
struct exp_and;

template<bool Arg>
struct exp_and<Arg>
{
	static const bool Value = Arg;
};

template<bool Arg, bool ...Args>
struct exp_and<Arg, Args...>
{
	static const bool Value = Arg && exp_and<Args...>::Value;
};

} }



#endif /* AND_H_ */
