/*
 *  hamming.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.27.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "hamming.h"
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
hamming_init (hamming_t *hamming, uint32_t size, double offset, opool_t *pool)
{
	int error;
	
	if (unlikely(hamming == NULL))
		LOG_ERROR_AND_RETURN(-1, "null hamming_t");
	
	if (unlikely(size == 0 || size > 10000))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)hamming, "dsp-window-hamming", (cobject_destroy_func)hamming_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	if (unlikely(NULL == (hamming->window = (double*)malloc(sizeof(double) * size))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc, %s", strerror(errno));
	
	hamming->size = size;
	hamming->offset = offset;
	
	hamming->dsp.__feed = (__dsp_feed_func)hamming_feed;
	hamming->dsp.__reset = (__dsp_reset_func)hamming_reset;
	
	for (uint32_t i = 0; i < size; ++i)
		hamming->window[i] = 0.54 - 0.46 * cos((2*M_PI*i)/size);
//	hamming->window[i] = 0.5 * (1. - cos((2*M_PI*i) / (size-1)));
	
	return 0;
}

/**
 *
 *
 */
int
hamming_destroy (hamming_t *hamming)
{
	int error;
	double *window;
	
	if (unlikely(hamming == NULL))
		LOG_ERROR_AND_RETURN(-1, "null hamming_t");
	
	window = hamming->window;
	
	if (window != NULL && ATOMIC_CASPTR_BARRIER(window, NULL, (void**)&hamming->window))
		free(window);
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)hamming))))
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
hamming_feed (hamming_t *hamming, uint32_t *size, double *data)
{
	uint32_t i, s;
	double *w, *d, o;
	
	if (unlikely(hamming == NULL))
		LOG_ERROR_AND_RETURN(-1, "null hamming_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	if (unlikely(*size != hamming->size * sizeof(double)))
		LOG_ERROR_AND_RETURN(-4, "mismatched sizes (size=%u, hamming->size=%lu)", *size, (hamming->size*sizeof(double)));
	
	s = hamming->size;
	w = hamming->window;
	o = hamming->offset;
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
hamming_reset (hamming_t *hamming)
{
	if (unlikely(hamming == NULL))
		LOG_ERROR_AND_RETURN(-1, "null hamming_t");
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline hamming_t*
hamming_retain (hamming_t *hamming)
{
	if (unlikely(hamming == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null hamming_t");
	
	return (hamming_t*)dsp_retain((dsp_t*)hamming);
}

/**
 *
 *
 */
inline void
hamming_release (hamming_t *hamming)
{
	if (unlikely(hamming == NULL))
		LOG_ERROR_AND_RETURN(, "null hamming_t");
	
	dsp_release((dsp_t*)hamming);
}

