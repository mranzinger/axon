/*
 * File description: can_stream_out.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef CAN_STREAM_OUT_H_
#define CAN_STREAM_OUT_H_

#include <iosfwd>
#include <type_traits>

#include "detail/sfinae_base.h"

namespace axon { namespace util {

detail::sfinae_base::no operator<<(const std::ostream &, const detail::sfinae_base::any_t &);

template<typename T>
struct can_stream_out
	: detail::sfinae_base
{
	template<typename U>
	static yes test(const U&);

	static no test(no);

	static T *s_t;

	static const bool Value = std::is_same<yes, decltype(test( *(std::ostream*)nullptr << *s_t ))>::value;
};



} }



#endif /* CAN_STREAM_OUT_H_ */
