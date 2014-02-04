/*
 * File description: sfinae_base.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef SFINAE_BASE_H_
#define SFINAE_BASE_H_

namespace axon { namespace util { namespace detail {

struct sfinae_base
{
	struct yes { char m[7]; };
	struct no { char m[5]; };

	struct any_t {
		template<typename U> any_t(const U &);
	};
};

} } }



#endif /* SFINAE_BASE_H_ */
