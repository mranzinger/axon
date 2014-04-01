/*
 * return_type.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef RETURN_TYPE_H_
#define RETURN_TYPE_H_

#include <type_traits>
#include <functional>

namespace axon { namespace util {

template<typename T>
struct function_traits;

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
// we specialize for pointers to member function
{
    enum { arity = sizeof...(Args) };
    // arity is the number of arguments.

    typedef ReturnType result_type;

    template <size_t i>
    struct arg
    {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        // the i-th argument is equivalent to the i-th tuple element of a tuple
        // composed of those arguments.
    };
};

template<typename ReturnType, typename ...Args>
struct function_traits<ReturnType (*)(Args...)>
{
	enum { arity = sizeof...(Args) };
    // arity is the number of arguments.

    typedef ReturnType result_type;

    template <size_t i>
    struct arg
    {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        // the i-th argument is equivalent to the i-th tuple element of a tuple
        // composed of those arguments.
    };
};

template<typename Ret, typename ...Args>
struct function_traits<std::function<Ret (Args...)>>
{
	enum { arity = sizeof...(Args) };

	typedef Ret result_type;

	template<size_t i>
	struct arg
	{
		typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
	};
};

// For generic types, directly use the result of the signature of its 'operator()'
template <typename T>
struct function_traits
    : public function_traits<decltype(&T::operator())>
{};

/*namespace {

template<typename F, typename Ret, typename ...Args>
Ret rt_helper(Ret (F::*)(Args...));

template<typename F, typename Ret, typename ...Args>
Ret rt_helper(Ret (F::*)(Args...) const);

}*/

template<typename F>
struct return_type
{
	typedef typename function_traits<F>::result_type type;
};

template<typename F>
struct return_type<F&>
{
	typedef typename function_traits<F>::result_type type;
};

} }



#endif /* RETURN_TYPE_H_ */
