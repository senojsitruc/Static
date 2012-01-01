/*
 *  int2double.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "int2double.h"
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
int2double_init (int2double_t *int2double, opool_t *pool)
{
	int error;
	
	if (unlikely(int2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null int2double_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)int2double, "dsp-type-int2double", (cobject_destroy_func)int2double_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	int2double->dsp.__feed = (__dsp_feed_func)int2double_feed;
	int2double->dsp.__reset = (__dsp_reset_func)int2double_reset;
	
	return 0;
}

/**
 *
 *
 */
int
int2double_destroy (int2double_t *int2double)
{
	int error;
	
	if (unlikely(int2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null int2double_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)int2double))))
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
int2double_feed (int2double_t *int2double, uint32_t *size, double *data)
{
	uint32_t i, count;
	double *dptr = data;
	int32_t *iptr = (int32_t*)data;
	
	if (unlikely(int2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null int2double_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	count = *size / sizeof(double);
	dptr += count - 1;
	iptr += count - 1;
	
	for (i = 0; i < count; ++i) {
		*dptr = (double)*iptr;
		
		dptr--;
		iptr--;
	}
	
	return 0;
}

/**
 *
 *
 */
int
int2double_reset (int2double_t *int2double)
{
	if (unlikely(int2double == NULL))
		LOG_ERROR_AND_RETURN(-1, "null int2double_t");
	
	return 0;
}	



#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline int2double_t*
int2double_retain (int2double_t *int2double)
{
	if (unlikely(int2double == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null int2double_t");
	
	return (int2double_t*)dsp_retain((dsp_t*)int2double);
}

/**
 *
 *
 */
inline void
int2double_release (int2double_t *int2double)
{
	if (unlikely(int2double == NULL))
		LOG_ERROR_AND_RETURN(, "null int2double_t");
	
	dsp_release((dsp_t*)int2double);
}
