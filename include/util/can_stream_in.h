/*
 * File description: can_stream_in.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef CAN_STREAM_IN_H_
#define CAN_STREAM_IN_H_

#include <iosfwd>
#include <type_traits>

#include "detail/sfinae_base.h"

namespace axon { namespace util {

detail::sfinae_base::no operator>>(const std::istream &, const detail::sfinae_base::any_t &);

template<typename T>
struct can_stream_in
	: detail::sfinae_base
{
	template<typename U>
	static yes test(const U&);

	static no test(no);

	static T s_t;

	static const bool Value = std::is_same<yes, decltype(test( *(std::istream*)nullptr >> s_t ))>::value;
};

} }

#endif /* CAN_STREAM_IN_H_ */
