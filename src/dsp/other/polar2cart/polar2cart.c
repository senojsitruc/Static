/*
 *  polar2cart.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "polar2cart.h"
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
polar2cart_init (polar2cart_t *polar2cart, opool_t *pool)
{
	int error;
	
	if (unlikely(polar2cart == NULL))
		LOG_ERROR_AND_RETURN(-1, "null polar2cart_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)polar2cart, "dsp-other-polar2cart", (cobject_destroy_func)polar2cart_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	polar2cart->dsp.__feed = (__dsp_feed_func)polar2cart_feed;
	polar2cart->dsp.__reset = (__dsp_reset_func)polar2cart_reset;
	
	return 0;
}

/**
 *
 *
 */
int
polar2cart_destroy (polar2cart_t *polar2cart)
{
	int error;
	
	if (unlikely(polar2cart == NULL))
		LOG_ERROR_AND_RETURN(-1, "null polar2cart_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)polar2cart))))
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
polar2cart_feed (polar2cart_t *polar2cart, uint32_t *size, double *data)
{
	uint32_t i, j, count;
	double r, t;
	
	if (unlikely(polar2cart == NULL))
		LOG_ERROR_AND_RETURN(-1, "null polar2cart_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	count = *size / sizeof(double);
	
	for (i=0, j=0; j < count; ++i, j+=2) {
		r = *(data+j+0);
		t = *(data+j+1);
		
		*(data+j+0) = r * cos( t );
		*(data+j+1) = r * sin( t );
	}
	
	return 0;
}

/**
 *
 *
 */
int
polar2cart_reset (polar2cart_t *polar2cart)
{
	if (unlikely(polar2cart == NULL))
		LOG_ERROR_AND_RETURN(-1, "null polar2cart_t");
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline polar2cart_t*
polar2cart_retain (polar2cart_t *polar2cart)
{
	if (unlikely(polar2cart == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null polar2cart_t");
	
	return (polar2cart_t*)dsp_retain((dsp_t*)polar2cart);
}

/**
 *
 *
 */
inline void
polar2cart_release (polar2cart_t *polar2cart)
{
	if (unlikely(polar2cart == NULL))
		LOG_ERROR_AND_RETURN(, "null polar2cart_t");
	
	dsp_release((dsp_t*)polar2cart);
}
