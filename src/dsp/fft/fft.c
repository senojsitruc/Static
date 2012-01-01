/*
 *  fft.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.27.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "fft.h"
#include "../../misc/logger.h"
#include "../../misc/mem/cobject.h"
#include <stdlib.h>





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
fft_init (fft_t *fft, uint32_t size, fft_direction direction, fft_type type, char *name, cobject_destroy_func destroy, opool_t *pool)
{
	int error;
	
	if (unlikely(fft == NULL))
		LOG_ERROR_AND_RETURN(-1, "null fft_t");
	
	if (unlikely(size == 0))
		LOG_ERROR_AND_RETURN(-2, "invalid size (0)");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)fft, name, destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	fft->size = size;
	fft->runs = 0;
	fft->direction = direction;
	fft->type = type;
	
	fft->dsp.__feed = (__dsp_feed_func)fft_feed;
	fft->dsp.__reset = (__dsp_reset_func)fft_reset;
	
	return 0;
}

/**
 *
 *
 */
int
fft_destroy (fft_t *fft)
{
	int error;
	
	if (unlikely(fft == NULL))
		LOG_ERROR_AND_RETURN(-1, "null fft_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)fft))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
double*
fft_data_in (fft_t *fft)
{
	if (unlikely(fft == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null fft_t");
	
	return fft->data_in;
}

/**
 *
 */
double*
fft_data_out (fft_t *fft)
{
	if (unlikely(fft == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null fft_t");
	
	return fft->data_out;
}





#pragma mark -
#pragma mark run

/**
 *
 *
 */
inline int
fft_feed (fft_t *fft, uint32_t *size, double *data)
{
	if (unlikely(fft == NULL))
		LOG_ERROR_AND_RETURN(-1, "null fft_t");
	
	if (unlikely(fft->__feed == NULL))
		LOG_ERROR_AND_RETURN(-2, "no 'feed' callback defined");
	
	fft->runs++;
	
	return (*fft->__feed)(fft, size, data);
}

/**
 *
 *
 */
inline int
fft_reset (fft_t *fft)
{
	if (unlikely(fft == NULL))
		LOG_ERROR_AND_RETURN(-1, "null fft_t");
	
	if (unlikely(fft->__reset == NULL))
		LOG_ERROR_AND_RETURN(-3, "no 'reset' callback defined");
	
	return (*fft->__reset)(fft);
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline fft_t*
fft_retain (fft_t *fft)
{
	if (unlikely(fft == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null fft_t");
	
	return (fft_t*)dsp_retain((dsp_t*)fft);
}

/**
 *
 *
 */
inline void
fft_release (fft_t *fft)
{
	if (unlikely(fft == NULL))
		LOG_ERROR_AND_RETURN(, "null fft_t");
	
	dsp_release((dsp_t*)fft);
}
