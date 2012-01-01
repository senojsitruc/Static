/*
 *  cpx2real.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.27.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "cpx2real.h"
#include "../../../device/device.h"
#include "../../../misc/atomic.h"
#include "../../../misc/logger.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
cpx2real_init (cpx2real_t *cpx2real, opool_t *pool)
{
	int error;
	
	if (unlikely(cpx2real == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cpx2real_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)cpx2real, "dsp-other-cpx2real", (cobject_destroy_func)cpx2real_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	cpx2real->dsp.__feed = (__dsp_feed_func)cpx2real_feed;
	cpx2real->dsp.__reset = (__dsp_reset_func)cpx2real_reset;
	
	return 0;
}

/**
 *
 *
 */
int
cpx2real_destroy (cpx2real_t *cpx2real)
{
	int error;
	
	if (unlikely(cpx2real == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cpx2real_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)cpx2real))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
cpx2real_feed (cpx2real_t *cpx2real, uint32_t *size, double *data)
{
	double *dsrc, *ddst, phase_i, phase_q;
	size_t i, count;
	
	if (unlikely(cpx2real == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cpx2real_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	dsrc = data;
	ddst = data;
	count = sizeof(double) / *size;
	
	for (i = 0; i < count; i+=2) {
		phase_i = *(dsrc+0);
		phase_q = *(dsrc+1);
		
		*ddst = sqrt((phase_i*phase_i) + (phase_q*phase_q));
		
		dsrc += 2;
		ddst += 1;
	} 
	
	// the amount of meaningful data in "data" is now half what it was previously
	*size = *size / 2;
	
	return 0;
}

/**
 *
 *
 */
int
cpx2real_reset (cpx2real_t *cpx2real)
{
	if (unlikely(cpx2real == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cpx2real_t");
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline cpx2real_t*
cpx2real_retain (cpx2real_t *cpx2real)
{
	if (unlikely(cpx2real == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null cpx2real_t");
	
	return (cpx2real_t*)dsp_retain((dsp_t*)cpx2real);
}

/**
 *
 *
 */
inline void
cpx2real_release (cpx2real_t *cpx2real)
{
	if (unlikely(cpx2real == NULL))
		LOG_ERROR_AND_RETURN(, "null cpx2real_t");
	
	dsp_release((dsp_t*)cpx2real);
}

