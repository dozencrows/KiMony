/*
 * util.h
 *
 *  Created on: 25 May 2015
 *      Author: ntuckett
 */

#ifndef UTIL_H_
#define UTIL_H_

// Ensure the function is copied to RAM and executed from there
#define RAM_FUNCTION __attribute__((section(".ram_code"), long_call, noinline))

#endif /* UTIL_H_ */
