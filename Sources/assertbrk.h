/*
 * assertbrk.h
 *
 *  Created on: 15 Jun 2015
 *      Author: ntuckett
 */

#ifndef ASSERTBRK_H_
#define ASSERTBRK_H_

#if defined(_DEBUG)
#define ASSERTBRK(x) if (!(x)) __asm("bkpt #0")
#else
#define ASSERTBRK(x)
#endif

#endif /* ASSERTBRK_H_ */
