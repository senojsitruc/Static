/*
 *  cart2polar.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "cart2polar.h"
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
cart2polar_init (cart2polar_t *cart2polar, opool_t *pool)
{
	int error;
	
	if (unlikely(cart2polar == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cart2polar_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)cart2polar, "dsp-other-cart2polar", (cobject_destroy_func)cart2polar_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	cart2polar->dsp.__feed = (__dsp_feed_func)cart2polar_feed;
	cart2polar->dsp.__reset = (__dsp_reset_func)cart2polar_reset;
	
	return 0;
}

/**
 *
 *
 */
int
cart2polar_destroy (cart2polar_t *cart2polar)
{
	int error;
	
	if (unlikely(cart2polar == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cart2polar_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)cart2polar))))
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
cart2polar_feed (cart2polar_t *cart2polar, uint32_t *size, double *data)
{
	uint32_t i, j, count;
	double x, y;
	
	if (unlikely(cart2polar == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cart2polar_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	count = *size / sizeof(double);
	
	for (i=0, j=0; j <count; ++i, j+=2) {
		x = *(data+j+0);
		y = *(data+j+1);
		
		*(data+j+0) = sqrt((x*x) + (y*y));
		*(data+j+1) = atan2(x,y);
	}
	
	return 0;
}

/**
 *
 *
 */
int
cart2polar_reset (cart2polar_t *cart2polar)
{
	if (unlikely(cart2polar == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cart2polar_t");
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline cart2polar_t*
cart2polar_retain (cart2polar_t *cart2polar)
{
	if (unlikely(cart2polar == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null cart2polar_t");
	
	return (cart2polar_t*)dsp_retain((dsp_t*)cart2polar);
}

/**
 *
 *
 */
inline void
cart2polar_release (cart2polar_t *cart2polar)
{
	if (unlikely(cart2polar == NULL))
		LOG_ERROR_AND_RETURN(, "null cart2polar_t");
	
	dsp_release((dsp_t*)cart2polar);
}
