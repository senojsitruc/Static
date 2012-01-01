/*
 *  baseband.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.12.03.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "baseband.h"
#include "../../../misc/atomic.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
baseband_init (baseband_t *baseband, uint32_t frequency, opool_t *pool)
{
	int error;
	
	if (unlikely(baseband == NULL))
		LOG_ERROR_AND_RETURN(-1, "null baseband_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)baseband, "dsp-other-baseband", (cobject_destroy_func)baseband_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	baseband->frequency = frequency;
	
	baseband->dsp.__feed = (__dsp_feed_func)baseband_feed;
	baseband->dsp.__reset = (__dsp_reset_func)baseband_reset;
	
	return 0;
}

/**
 *
 *
 */
int
baseband_destroy (baseband_t *baseband)
{
	int error;
	
	if (unlikely(baseband == NULL))
		LOG_ERROR_AND_RETURN(-1, "null baseband_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)baseband))))
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
baseband_feed (baseband_t *baseband, uint32_t *size, double *data)
{
	size_t j, count;
	double i, q, f, t, *phase_i, *phase_q;
	
	if (unlikely(baseband == NULL))
		LOG_ERROR_AND_RETURN(-1, "null baseband_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	count = *size / sizeof(double);
	phase_i = data;
	phase_q = data + 1;
	f = baseband->frequency;
	t = 1.;
	
	for (j = 1; j < count; j += 2) {
		i = *phase_i;
		q = *phase_q;
		
		i *=  cos(M_2_PI * f * t);
		q *= -sin(M_2_PI * f * t);
		
		*phase_i = i;
		*phase_q = q;
		
		t += 1.;
		phase_i += 2;
		phase_q += 2;
	}
	
	return 0;
}

/**
 *
 *
 */
int
baseband_reset (baseband_t *baseband)
{
	if (unlikely(baseband == NULL))
		LOG_ERROR_AND_RETURN(-1, "null baseband_t");
	
	// anything to reset?
	
	return 0;
}	





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline baseband_t*
baseband_retain (baseband_t *baseband)
{
	if (unlikely(baseband == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null baseband_t");
	
	return (baseband_t*)dsp_retain((dsp_t*)baseband);
}

/**
 *
 *
 */
inline void
baseband_release (baseband_t *baseband)
{
	if (unlikely(baseband == NULL))
		LOG_ERROR_AND_RETURN(, "null baseband_t");
	
	dsp_release((dsp_t*)baseband);
}

