/*
 *  flattop.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "flattop.h"
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
flattop_init (flattop_t *flattop, uint32_t size, double offset, opool_t *pool)
{
	int error;
	
	if (unlikely(flattop == NULL))
		LOG_ERROR_AND_RETURN(-1, "null flattop_t");
	
	if (unlikely(size == 0 || size > 10000))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)flattop, "dsp-window-flattop", (cobject_destroy_func)flattop_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	if (unlikely(NULL == (flattop->window = (double*)malloc(sizeof(double) * size))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc, %s", strerror(errno));
	
	flattop->size = size;
	flattop->offset = offset;
	
	flattop->dsp.__feed = (__dsp_feed_func)flattop_feed;
	flattop->dsp.__reset = (__dsp_reset_func)flattop_reset;
	
	for (uint32_t i = 0; i < size; ++i)
		flattop->window[i] = 1. - 1.93  * cos((2*M_PI*i)/(size-1))
		                        + 1.29  * cos((4*M_PI*i)/(size-1))
		                        - 0.388 * cos((6*M_PI*i)/(size-1))
		                        + 0.32  * cos((8*M_PI*i)/(size-1));
	
	return 0;
}

/**
 *
 *
 */
int
flattop_destroy (flattop_t *flattop)
{
	int error;
	double *window;
	
	if (unlikely(flattop == NULL))
		LOG_ERROR_AND_RETURN(-1, "null flattop_t");
	
	window = flattop->window;
	
	if (window != NULL && ATOMIC_CASPTR_BARRIER(window, NULL, (void**)&flattop->window))
		free(window);
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)flattop))))
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
flattop_feed (flattop_t *flattop, uint32_t *size, double *data)
{
	uint32_t i, s;
	double *w, *d, o;
	
	if (unlikely(flattop == NULL))
		LOG_ERROR_AND_RETURN(-1, "null flattop_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	if (unlikely(*size != flattop->size * sizeof(double)))
		LOG_ERROR_AND_RETURN(-4, "mismatched sizes (size=%u, flattop->size=%lu)", *size, (flattop->size*sizeof(double)));
	
	s = flattop->size;
	w = flattop->window;
	o = flattop->offset;
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
flattop_reset (flattop_t *flattop)
{
	if (unlikely(flattop == NULL))
		LOG_ERROR_AND_RETURN(-1, "null flattop_t");
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline flattop_t*
flattop_retain (flattop_t *flattop)
{
	if (unlikely(flattop == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null flattop_t");
	
	return (flattop_t*)dsp_retain((dsp_t*)flattop);
}

/**
 *
 *
 */
inline void
flattop_release (flattop_t *flattop)
{
	if (unlikely(flattop == NULL))
		LOG_ERROR_AND_RETURN(, "null flattop_t");
	
	dsp_release((dsp_t*)flattop);
}

