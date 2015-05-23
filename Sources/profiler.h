/*
 * profiler.h
 *
 *  Created on: 22 May 2015
 *      Author: ntuckett
 */

#ifndef PROFILER_H_
#define PROFILER_H_

#ifdef PROFILING
#include <stdio.h>
#include <stdint.h>
#include "systick.h"

#define PROFILE_CATEGORY(category) uint32_t ctr_##category; uint32_t calls_##category
#define PROFILE_BEGIN memset(&profilerMetrics, 0, sizeof(profilerMetrics)); sysTickStartCycleCount()
#define PROFILE_ENTER(category) uint32_t profileMark_##category = sysTickGetCycleCount()
#define PROFILE_EXIT(category) profilerMetrics.ctr_##category += sysTickGetCycleCount() - profileMark_##category; profilerMetrics.calls_##category++
#define PROFILE_END sysTickStopCycleCount()
#define PROFILE_REPORT(category) printf("%s: %d, %d\n", #category, profilerMetrics.ctr_##category, profilerMetrics.calls_##category)

typedef struct _ProfilerMetrics {
	PROFILE_CATEGORY(drawlist);
	PROFILE_CATEGORY(render);
	PROFILE_CATEGORY(scanline);
	PROFILE_CATEGORY(clearline);
	PROFILE_CATEGORY(activationCheck);
	PROFILE_CATEGORY(primitives);
	PROFILE_CATEGORY(hline);
	PROFILE_CATEGORY(vline);
	PROFILE_CATEGORY(rect);
	PROFILE_CATEGORY(text);
	PROFILE_CATEGORY(image);
	PROFILE_CATEGORY(profileOuter);
	PROFILE_CATEGORY(profileInner);
	PROFILE_CATEGORY(blit);
} ProfilerMetrics;

extern ProfilerMetrics profilerMetrics;

#else

#define PROFILE_BEGIN
#define PROFILE_ENTER(category)
#define PROFILE_EXIT(category)
#define PROFILE_END
#define PROFILE_REPORT(category)

#endif



#endif /* PROFILER_H_ */
