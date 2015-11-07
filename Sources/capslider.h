/*
 * capslider.h
 *
 *  Created on: 7 Nov 2015
 *      Author: ntuckett
 */

#ifndef CAPSLIDER_H_
#define CAPSLIDER_H_

#include <stdint.h>

extern void capSliderInit();
extern void capSliderStartRead(int channel);
extern int capSliderReadDone();
extern int capSliderValue();

#endif /* CAPSLIDER_H_ */
