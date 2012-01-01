/*
 *  blackman.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "blackman.h"
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
blackman_init (blackman_t *blackman, uint32_t size, double offset, opool_t *pool)
{
	int error;
	
	if (unlikely(blackman == NULL))
		LOG_ERROR_AND_RETURN(-1, "null blackman_t");
	
	if (unlikely(size == 0 || size > 10000))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)blackman, "dsp-window-blackman", (cobject_destroy_func)blackman_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	if (unlikely(NULL == (blackman->window = (double*)malloc(sizeof(double) * size))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc, %s", strerror(errno));
	
	blackman->size = size;
	blackman->offset = offset;
	
	blackman->dsp.__feed = (__dsp_feed_func)blackman_feed;
	blackman->dsp.__reset = (__dsp_reset_func)blackman_reset;
	
	for (uint32_t i = 0; i < size; ++i)
		blackman->window[i] = 0.42 - 0.5 * cos(2*M_PI*i/size) + 0.08 * cos(4*M_PI*i/size);
	
	return 0;
}

/**
 *
 *
 */
int
blackman_destroy (blackman_t *blackman)
{
	int error;
	double *window;
	
	if (unlikely(blackman == NULL))
		LOG_ERROR_AND_RETURN(-1, "null blackman_t");
	
	window = blackman->window;
	
	if (window != NULL && ATOMIC_CASPTR_BARRIER(window, NULL, (void**)&blackman->window))
		free(window);
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)blackman))))
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
blackman_feed (blackman_t *blackman, uint32_t *size, double *data)
{
	uint32_t i, s;
	double *w, *d, o;
	
	if (unlikely(blackman == NULL))
		LOG_ERROR_AND_RETURN(-1, "null blackman_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	if (unlikely(*size != blackman->size * sizeof(double)))
		LOG_ERROR_AND_RETURN(-4, "mismatched sizes (size=%u, blackman->size=%lu)", *size, (blackman->size*sizeof(double)));
	
	s = blackman->size;
	w = blackman->window;
	o = blackman->offset;
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
blackman_reset (blackman_t *blackman)
{
	if (unlikely(blackman == NULL))
		LOG_ERROR_AND_RETURN(-1, "null blackman_t");
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline blackman_t*
blackman_retain (blackman_t *blackman)
{
	if (unlikely(blackman == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null blackman_t");
	
	return (blackman_t*)dsp_retain((dsp_t*)blackman);
}

/**
 *
 *
 */
inline void
blackman_release (blackman_t *blackman)
{
	if (unlikely(blackman == NULL))
		LOG_ERROR_AND_RETURN(, "null blackman_t");
	
	dsp_release((dsp_t*)blackman);
}
