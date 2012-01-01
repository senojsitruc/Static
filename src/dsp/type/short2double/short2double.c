/*
 *  short2double.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "short2double.h"
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
short2double_init (short2double_t *short2double, opool_t *pool)
{
	int error;
	
	if (unlikely(short2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null short2double_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)short2double, "dsp-type-short2double", (cobject_destroy_func)short2double_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	short2double->dsp.__feed = (__dsp_feed_func)short2double_feed;
	short2double->dsp.__reset = (__dsp_reset_func)short2double_reset;
	
	return 0;
}

/**
 *
 *
 */
int
short2double_destroy (short2double_t *short2double)
{
	int error;
	
	if (unlikely(short2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null short2double_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)short2double))))
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
short2double_feed (short2double_t *short2double, uint32_t *size, int16_t *data)
{
	uint32_t i, count;
	double *dptr = (double*)data;
	int16_t *sptr = data;
	
	if (unlikely(short2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null short2double_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	count = *size / sizeof(uint16_t);
	dptr += count - 1;
	sptr += count - 1;
	
	for (i = 0; i < count; ++i) {
		*dptr = (double)*sptr;
		
		dptr--;
		sptr--;
	}
	
	*size = count * sizeof(double);
	
	return 0;
}

/**
 *
 *
 */
int
short2double_reset (short2double_t *short2double)
{
	if (unlikely(short2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null short2double_t");
	
	return 0;
}	



#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline short2double_t*
short2double_retain (short2double_t *short2double)
{
	if (unlikely(short2double == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null short2double_t");
	
	return (short2double_t*)dsp_retain((dsp_t*)short2double);
}

/**
 *
 *
 */
inline void
short2double_release (short2double_t *short2double)
{
	if (unlikely(short2double == NULL))
		LOG_ERROR_AND_RETURN(, "null short2double_t");
	
	dsp_release((dsp_t*)short2double);
}
