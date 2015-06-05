//=======================================================================
// Copyright Nicholas Tuckett 2015.
// Distributed under the MIT License.
// (See accompanying file license.txt or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*
 * keymatrix.h
 *
 *  Created on: 13 Apr 2015
 *      Author: ntuckett
 */

#ifndef KEYMATRIX_H_
#define KEYMATRIX_H_

#include <stdint.h>

#define KEY_MATRIX_MASK		0x0000ffff
#define KEY_NONMATRIX_MASK	0xffff0000

extern void keyMatrixInit();
extern void keyMatrixClearInterrupt();
extern int keyMatrixCheckInterrupt();
extern uint32_t keyMatrixPoll();
extern uint32_t keyNonMatrixPoll();
extern void testKeyMatrix();


#endif /* KEYMATRIX_H_ */
