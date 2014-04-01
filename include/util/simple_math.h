/*
 * File description: simple_math.h
 * Author information: Mike Ranzinger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef SIMPLE_MATH_H_
#define SIMPLE_MATH_H_

#include <algorithm>
#include <type_traits>

namespace axon { namespace communication {

template<typename T>
auto Square(const T &val) -> decltype(val * val)
{
	return val * val;
}

template<typename IterType, typename Extractor>
auto Sum(IterType iter, IterType end, Extractor ext) -> typename std::remove_const<typename std::remove_reference<decltype(*iter)>::type>::type
{
	typename std::remove_const<typename std::remove_reference<decltype(*iter)>::type>::type
			sum = 0;

	for (; iter != end; ++iter)
		sum += ext(*iter);

	return sum;
}

template<typename IterType>
auto Sum(IterType iter, IterType end) -> typename std::remove_const<typename std::remove_reference<decltype(*iter)>::type>::type
{
	return Sum(iter, end, [](decltype(*iter) val) { return val; });
}

template<typename IterType, typename Extractor>
double Mean(IterType iter, IterType end, Extractor ext)
{
	double sum = Sum(iter, end, ext);

	return sum / (end - iter);
}

template<typename IterType>
double Mean(IterType iter, IterType end)
{
	return Mean(iter, end, [](decltype(*iter) val) { return val; });
}

template<typename IterType, typename Extractor>
double StdDev(IterType iter, IterType end, Extractor ext)
{
	double mean = Mean(iter, end, ext);

	double stdsum = Sum(iter, end,
			[mean] (decltype(*iter) val)
			{
				return Square(val - mean);
			});

	return std::sqrt(stdsum / (end - iter));

}

template<typename IterType>
double StdDev(IterType iter, IterType end)
{
	return StdDev(iter, end, [](decltype(*iter) val) { return val; });
}

template<typename IterType>
IterType FindMin(IterType iter, IterType end)
{
	if (iter == end)
		return end;

	auto cmin = *iter;
	auto imin = iter;
	++iter;

	for (; iter != end; ++iter)
	{
		if (*iter < cmin)
		{
			cmin = *iter;
			imin = iter;
		}
	}

	return imin;
}

} }



#endif /* SIMPLE_MATH_H_ */
