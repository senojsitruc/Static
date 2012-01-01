/*
 *  double2sint16.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.29.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "double2sint16.h"
#include "../../../misc/atomic.h"
#include "../../../misc/logger.h"
#include <errno.h>
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
double2sint16_init (double2sint16_t *double2sint16, opool_t *pool)
{
	int error;
	
	if (unlikely(double2sint16 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null double2sint16_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)double2sint16, "dsp-other-double2sint16", (cobject_destroy_func)double2sint16_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	double2sint16->dsp.__feed = (__dsp_feed_func)double2sint16_feed;
	double2sint16->dsp.__reset = (__dsp_reset_func)double2sint16_reset;
	
	return 0;
}

/**
 *
 *
 */
int
double2sint16_destroy (double2sint16_t *double2sint16)
{
	int error;
	
	if (unlikely(double2sint16 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null double2sint16_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)double2sint16))))
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
double2sint16_feed (double2sint16_t *double2sint16, uint32_t *size, double *data)
{
	size_t i, count;
	double *src;
	int16_t *dst;
	
	if (unlikely(double2sint16 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null double2sint16_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	count = *size / sizeof(double);
	src = data;
	dst = (int16_t*)data;
	
	for (i = 0; i < count; ++i, ++src, ++dst)
		*src = (int16_t)*data;
	
	*size = (uint32_t)(sizeof(int16_t) * count);
	
	return 0;
}

/**
 *
 *
 */
int
double2sint16_reset (double2sint16_t *double2sint16)
{
	if (unlikely(double2sint16 == NULL))
		LOG_ERROR_AND_RETURN(-1, "null double2sint16_t");
	
	return 0;
}	





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline double2sint16_t*
double2sint16_retain (double2sint16_t *double2sint16)
{
	if (unlikely(double2sint16 == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null double2sint16_t");
	
	return (double2sint16_t*)dsp_retain((dsp_t*)double2sint16);
}

/**
 *
 *
 */
inline void
double2sint16_release (double2sint16_t *double2sint16)
{
	if (unlikely(double2sint16 == NULL))
		LOG_ERROR_AND_RETURN(, "null double2sint16_t");
	
	dsp_release((dsp_t*)double2sint16);
}

