/*
 *  smooth.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "smooth.h"
#include "../../../misc/atomic.h"
#include "../../../misc/logger.h"
#include <errno.h>
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
smooth_init (smooth_t *smooth, uint32_t period, opool_t *pool)
{
	int error;
	
	if (unlikely(smooth == NULL))
		LOG_ERROR_AND_RETURN(-1, "null smooth_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)smooth, "dsp-other-smooth", (cobject_destroy_func)smooth_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	smooth->period = period;
	
	smooth->dsp.__feed = (__dsp_feed_func)smooth_feed;
	smooth->dsp.__reset = (__dsp_reset_func)smooth_reset;
	
	return 0;
}

/**
 *
 *
 */
int
smooth_destroy (smooth_t *smooth)
{
	int error;
	
	if (unlikely(smooth == NULL))
		LOG_ERROR_AND_RETURN(-1, "null smooth_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)smooth))))
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
smooth_feed (smooth_t *smooth, uint32_t *size, double *data)
{
	uint32_t i, count;
	double pt0, pt1, pt2, pt3;
	
	if (unlikely(smooth == NULL))
		LOG_ERROR_AND_RETURN(-1, "null smooth_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	count = *size / sizeof(double);
	
	for (i = 0; i < count; ++i, ++data) {
		pt0 = *data;
		
		if (i < count-1) pt1 = *(data+1); else pt1 = 0;
		if (i < count-1) pt2 = *(data+2); else pt2 = 0;
		if (i < count-1) pt3 = *(data+3); else pt3 = 0;
		
		if (pt1 > pt0) {
		}
		else if (pt2 > pt0) {
			*(data+1) = 0.;
			data += 2;
			i += 1;
		}
		else if (pt3 > pt0) {
			*(data+1) = 0.;
			*(data+2) = 0.;
			data += 3;
			i += 2;
		}
	}
	
	return 0;
}

/**
 *
 *
 */
int
smooth_reset (smooth_t *smooth)
{
	if (unlikely(smooth == NULL))
		LOG_ERROR_AND_RETURN(-1, "null smooth_t");
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline smooth_t*
smooth_retain (smooth_t *smooth)
{
	if (unlikely(smooth == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null smooth_t");
	
	return (smooth_t*)dsp_retain((dsp_t*)smooth);
}

/**
 *
 *
 */
inline void
smooth_release (smooth_t *smooth)
{
	if (unlikely(smooth == NULL))
		LOG_ERROR_AND_RETURN(, "null smooth_t");
	
	dsp_release((dsp_t*)smooth);
}

