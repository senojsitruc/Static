/*
 *  triangle.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "triangle.h"
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
triangle_init (triangle_t *triangle, uint32_t size, double offset, opool_t *pool)
{
	int error;
	
	if (unlikely(triangle == NULL))
		LOG_ERROR_AND_RETURN(-1, "null triangle_t");
	
	if (unlikely(size == 0 || size > 10000))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)triangle, "dsp-window-triangle", (cobject_destroy_func)triangle_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	if (unlikely(NULL == (triangle->window = (double*)malloc(sizeof(double) * size))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc, %s", strerror(errno));
	
	triangle->size = size;
	triangle->offset = offset;
	
	triangle->dsp.__feed = (__dsp_feed_func)triangle_feed;
	triangle->dsp.__reset = (__dsp_reset_func)triangle_reset;
	
	for (uint32_t i = 0; i < size; ++i)
		triangle->window[i] = (2. / size) * ( (size/2.) - fabs(i - ((size-1)/2)) );
	
	return 0;
}

/**
 *
 *
 */
int
triangle_destroy (triangle_t *triangle)
{
	int error;
	double *window;
	
	if (unlikely(triangle == NULL))
		LOG_ERROR_AND_RETURN(-1, "null triangle_t");
	
	window = triangle->window;
	
	if (window != NULL && ATOMIC_CASPTR_BARRIER(window, NULL, (void**)&triangle->window))
		free(window);
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)triangle))))
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
triangle_feed (triangle_t *triangle, uint32_t *size, double *data)
{
	uint32_t i, s;
	double *w, *d, o;
	
	if (unlikely(triangle == NULL))
		LOG_ERROR_AND_RETURN(-1, "null triangle_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	if (unlikely(*size != triangle->size * sizeof(double)))
		LOG_ERROR_AND_RETURN(-4, "mismatched sizes (size=%u, triangle->size=%lu)", *size, (triangle->size*sizeof(double)));
	
	s = triangle->size;
	w = triangle->window;
	o = triangle->offset;
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
triangle_reset (triangle_t *triangle)
{
	if (unlikely(triangle == NULL))
		LOG_ERROR_AND_RETURN(-1, "null triangle_t");
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline triangle_t*
triangle_retain (triangle_t *triangle)
{
	if (unlikely(triangle == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null triangle_t");
	
	return (triangle_t*)dsp_retain((dsp_t*)triangle);
}

/**
 *
 *
 */
inline void
triangle_release (triangle_t *triangle)
{
	if (unlikely(triangle == NULL))
		LOG_ERROR_AND_RETURN(, "null triangle_t");
	
	dsp_release((dsp_t*)triangle);
}

