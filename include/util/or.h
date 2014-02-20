/*
 * File description: or.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef OR_H_
#define OR_H_

namespace axon { namespace util {

template<bool ...Args>
struct exp_or;

template<bool Arg>
struct exp_or<Arg>
{
	static const bool Value = Arg;
};

template<bool Arg, bool ...Args>
struct exp_or<Arg, Args...>
{
	static const bool Value = Arg || exp_or<Args...>::Value;
};

} }



#endif /* OR_H_ */
