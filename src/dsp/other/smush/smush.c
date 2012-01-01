/*
 *  smush.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.26.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "smush.h"
#include "../../../misc/atomic.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <math.h>
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
dspsmush_init (dspsmush_t *dspsmush, uint32_t width, opool_t *pool)
{
	int error;
	
	if (unlikely(dspsmush == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspsmush_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)dspsmush, "dsp-other-dspsmush", (cobject_destroy_func)dspsmush_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	dspsmush->width = width;
	
	dspsmush->dsp.__feed = (__dsp_feed_func)dspsmush_feed;
	dspsmush->dsp.__reset = (__dsp_reset_func)dspsmush_reset;
	
	return 0;
}

/**
 *
 *
 */
int
dspsmush_destroy (dspsmush_t *dspsmush)
{
	int error;
	
	if (unlikely(dspsmush == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspsmush_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)dspsmush))))
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
dspsmush_feed (dspsmush_t *dspsmush, uint32_t *size, double *data)
{
	size_t i, srcx, dstx, width, count;
	double *dsrc, *ddst;
	
	if (unlikely(dspsmush == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspsmush_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	width = dspsmush->width;
	count = *size / sizeof(double);
	dsrc = data;
	ddst = data;
	
	for (srcx=0, dstx=0; srcx < count;) {
		double max = *dsrc;
		
		dsrc++;
		srcx++;
		
		for (i = 1; srcx < count && dstx < width && i < (size_t)round((count-srcx)/(width-dstx)); ++i, ++dsrc, ++srcx) {
			if (*dsrc > max)
				max = *dsrc;
		}
		
		*ddst = max;
		ddst++;
		dstx++;
	}
	
	*size = (uint32_t)width * sizeof(double);
	
	return 0;
}

/**
 *
 *
 */
int
dspsmush_reset (dspsmush_t *dspsmush)
{
	if (unlikely(dspsmush == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspsmush_t");
	
	return 0;
}	





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline dspsmush_t*
dspsmush_retain (dspsmush_t *dspsmush)
{
	if (unlikely(dspsmush == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null dspsmush_t");
	
	return (dspsmush_t*)dsp_retain((dsp_t*)dspsmush);
}

/**
 *
 *
 */
inline void
dspsmush_release (dspsmush_t *dspsmush)
{
	if (unlikely(dspsmush == NULL))
		LOG_ERROR_AND_RETURN(, "null dspsmush_t");
	
	dsp_release((dsp_t*)dspsmush);
}

