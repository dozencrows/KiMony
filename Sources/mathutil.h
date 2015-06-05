//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * mathutil.h
 *
 *  Created on: 11 May 2015
 *      Author: ntuckett
 */

#ifndef MATHUTIL_H_
#define MATHUTIL_H_

#define SWAPNUM(a, b) a = a ^ b; b = a ^ b; a = a ^ b
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct _Point {
	uint16_t	x;
	uint16_t	y;
} Point;

#endif /* MATHUTIL_H_ */
