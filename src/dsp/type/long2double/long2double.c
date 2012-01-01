/*
 *  long2double.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "long2double.h"
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
long2double_init (long2double_t *long2double, opool_t *pool)
{
	int error;
	
	if (unlikely(long2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null long2double_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)long2double, "dsp-type-long2double", (cobject_destroy_func)long2double_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	long2double->dsp.__feed = (__dsp_feed_func)long2double_feed;
	long2double->dsp.__reset = (__dsp_reset_func)long2double_reset;
	
	return 0;
}

/**
 *
 *
 */
int
long2double_destroy (long2double_t *long2double)
{
	int error;
	
	if (unlikely(long2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null long2double_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)long2double))))
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
long2double_feed (long2double_t *long2double, uint32_t *size, double *data)
{
	uint32_t i, count;
	double *dptr = data;
	int64_t *lptr = (int64_t*)data;
	
	if (unlikely(long2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null long2double_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	count = *size / sizeof(double);
	dptr += count - 1;
	lptr += count - 1;
	
	for (i = 0; i < count; ++i) {
		*dptr = (double)*lptr;
		
		dptr--;
		lptr--;
	}
	
	return 0;
}

/**
 *
 *
 */
int
long2double_reset (long2double_t *long2double)
{
	if (unlikely(long2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null long2double_t");
	
	return 0;
}	



#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline long2double_t*
long2double_retain (long2double_t *long2double)
{
	if (unlikely(long2double == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null long2double_t");
	
	return (long2double_t*)dsp_retain((dsp_t*)long2double);
}

/**
 *
 *
 */
inline void
long2double_release (long2double_t *long2double)
{
	if (unlikely(long2double == NULL))
		LOG_ERROR_AND_RETURN(, "null long2double_t");
	
	dsp_release((dsp_t*)long2double);
}
