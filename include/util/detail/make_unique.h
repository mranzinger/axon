/*
 * make_unique.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef MAKE_UNIQUE_H_
#define MAKE_UNIQUE_H_

#define MAKE_UNIQUE_CC(Name, Count) Name ## Count
#define MAKE_UNIQUE_C(Name, Count) MAKE_UNIQUE_CC(Name, Count)
#define MAKE_UNIQUE(Name) MAKE_UNIQUE_C(Name, __COUNTER__)



#endif /* MAKE_UNIQUE_H_ */
