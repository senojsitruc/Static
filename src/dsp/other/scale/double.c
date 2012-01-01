/*
 *  double.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.28.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "double.h"
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
scaledouble_init (scaledouble_t *scaledouble, dspscale_mode mode, double src_hi, double src_lo, double dst_hi, double dst_lo, opool_t *pool)
{
	int error;
	
	if (unlikely(scaledouble == NULL))
		LOG_ERROR_AND_RETURN(-1, "null scaledouble_t");
	
	if (unlikely(src_lo >= src_hi))
		LOG_ERROR_AND_RETURN(-1, "source low (%f) >= source high (%f)", src_lo, src_hi);
	
	if (unlikely(dst_lo >= dst_hi))
		LOG_ERROR_AND_RETURN(-2, "destination low (%f) >= destination high (%f)", dst_lo, dst_hi);
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)scaledouble, "dsp-other-scaledouble", (cobject_destroy_func)scaledouble_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	scaledouble->mode = mode;
	
	scaledouble->src_hi = src_hi;
	scaledouble->src_lo = src_lo;
	scaledouble->dst_hi = dst_hi;
	scaledouble->dst_lo = dst_lo;
	
	scaledouble->src_range = src_hi - src_lo;
	scaledouble->dst_range = dst_hi - dst_lo;
	
	scaledouble->dsp.__feed = (__dsp_feed_func)scaledouble_feed;
	scaledouble->dsp.__reset = (__dsp_reset_func)scaledouble_reset;
	
	return 0;
}

/**
 *
 *
 */
int
scaledouble_destroy (scaledouble_t *scaledouble)
{
	int error;
	
	if (unlikely(scaledouble == NULL))
		LOG_ERROR_AND_RETURN(-1, "null scaledouble_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)scaledouble))))
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
scaledouble_feed (scaledouble_t *scaledouble, uint32_t *size, double *data)
{
	size_t i, count;
	double *dataptr, value, mul;
	double src_range, dst_range;
	double src_zero = 0.;
	double src_hi, src_lo, dst_hi, dst_lo;
	dspscale_mode mode;
	
	if (unlikely(scaledouble == NULL))
		LOG_ERROR_AND_RETURN(-1, "null scaledouble_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	dataptr = data;
	mode = scaledouble->mode;
	count = *size / sizeof(double);
	
	// source and destination ranges. if lo=100, hi=300, range=200 (ie, hi - lo)
	src_range = scaledouble->src_range;
	dst_range = scaledouble->dst_range;
	
	// local copies of the source and destination high and low values
	src_hi = scaledouble->src_hi;
	src_lo = scaledouble->src_lo;
	dst_hi = scaledouble->dst_hi;
	dst_lo = scaledouble->dst_lo;
	
	// automatic gain control?
	/*
	{
		for (i = 0; i < count; ++i, ++dataptr) {
			value = *dataptr;
			
			if (value < src_lo)
				src_lo = scaledouble->src_lo = value;
			
			if (value > src_hi)
				src_hi = scaledouble->src_hi = value;
		}
		
		dataptr = data;
	}
	*/
	
	// if the source low is negative and the source high is positive, we need to know where zero lies
	// relative to the range "high - low". if lo=-50, hi=+100, then zero=+0.33
	if (src_lo < 0. && src_hi > 0.)
		src_zero = src_range / fabs(src_lo);
	
	// linear
	if (DSPSCALE_MODE_LIN == mode)
	{
		// the source range is entirely positive
		if (src_zero == 0.) {
			for (i = 0; i < count; ++i, ++dataptr) {
				value = *dataptr;
				
				if (value > src_lo && value < src_hi)
					value = dst_lo + (dst_range * ((value - src_lo) / src_range));
				else if (value < src_lo)
					value = dst_lo;
				else if (value > src_hi)
					value = dst_hi;
				
				*dataptr = value;
			}
		}
		// the source range is not entirely positive (ie, partially negative)
		else {
			for (i = 0; i < count; ++i, ++dataptr) {
				value = *dataptr;
				
				if (value > src_lo && value < src_hi) {
					if (value < 0.)
						mul = src_zero * (value / src_lo);
					else if (value > 0.)
						mul = src_zero * (value / src_hi);
					else
						mul = src_zero;
					
					value = dst_lo + (dst_range * mul);
				}
				else if (value < src_lo)
					value = src_lo;
				else if (value > src_hi)
					value = src_hi;
				
				*dataptr = value;
			}
		}
	}
	else
		LOG3("unsupported mode, %d", (int)mode);
	
	return 0;
}

/**
 *
 *
 */
int
scaledouble_reset (scaledouble_t *scaledouble)
{
	if (unlikely(scaledouble == NULL))
		LOG_ERROR_AND_RETURN(-1, "null scaledouble_t");
	
	return 0;
}	





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline scaledouble_t*
scaledouble_retain (scaledouble_t *scaledouble)
{
	if (unlikely(scaledouble == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null scaledouble_t");
	
	return (scaledouble_t*)dsp_retain((dsp_t*)scaledouble);
}

/**
 *
 *
 */
inline void
scaledouble_release (scaledouble_t *scaledouble)
{
	if (unlikely(scaledouble == NULL))
		LOG_ERROR_AND_RETURN(, "null scaledouble_t");
	
	dsp_release((dsp_t*)scaledouble);
}
