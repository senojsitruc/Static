/*
 *  rectangle.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "rectangle.h"
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
rectangle_init (rectangle_t *rectangle, uint32_t size, double offset, opool_t *pool)
{
	int error;
	
	if (unlikely(rectangle == NULL))
		LOG_ERROR_AND_RETURN(-1, "null rectangle_t");
	
	if (unlikely(size == 0 || size > 10000))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)rectangle, "dsp-window-rectangle", (cobject_destroy_func)rectangle_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	if (unlikely(NULL == (rectangle->window = (double*)malloc(sizeof(double) * size))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc, %s", strerror(errno));
	
	rectangle->size = size;
	rectangle->offset = offset;
	
	rectangle->dsp.__feed = (__dsp_feed_func)rectangle_feed;
	rectangle->dsp.__reset = (__dsp_reset_func)rectangle_reset;
	
	for (uint32_t i = 0; i < size; ++i)
		rectangle->window[i] = 1.;
	
	return 0;
}

/**
 *
 *
 */
int
rectangle_destroy (rectangle_t *rectangle)
{
	int error;
	double *window;
	
	if (unlikely(rectangle == NULL))
		LOG_ERROR_AND_RETURN(-1, "null rectangle_t");
	
	window = rectangle->window;
	
	if (window != NULL && ATOMIC_CASPTR_BARRIER(window, NULL, (void**)&rectangle->window))
		free(window);
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)rectangle))))
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
rectangle_feed (rectangle_t *rectangle, uint32_t *size, double *data)
{
	uint32_t i, s;
	double *w, *d, o;
	
	if (unlikely(rectangle == NULL))
		LOG_ERROR_AND_RETURN(-1, "null rectangle_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	if (unlikely(*size != rectangle->size * sizeof(double)))
		LOG_ERROR_AND_RETURN(-4, "mismatched sizes (size=%u, rectangle->size=%lu)", *size, (rectangle->size*sizeof(double)));
	
	s = rectangle->size;
	w = rectangle->window;
	o = rectangle->offset;
//r = 0; // because sword
	d = data;
	
	for (i = 0; i < s; ++i) {
		*d = *d * (*w + o);
		
		w++;
		d++;
	}
	
	return 0;
}

/**
 *
 *
 */
int
rectangle_reset (rectangle_t *rectangle)
{
	if (unlikely(rectangle == NULL))
		LOG_ERROR_AND_RETURN(-1, "null rectangle_t");
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline rectangle_t*
rectangle_retain (rectangle_t *rectangle)
{
	if (unlikely(rectangle == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null rectangle_t");
	
	return (rectangle_t*)dsp_retain((dsp_t*)rectangle);
}

/**
 *
 *
 */
inline void
rectangle_release (rectangle_t *rectangle)
{
	if (unlikely(rectangle == NULL))
		LOG_ERROR_AND_RETURN(, "null rectangle_t");
	
	dsp_release((dsp_t*)rectangle);
}

