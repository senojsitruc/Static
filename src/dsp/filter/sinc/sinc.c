/*
 *  sinc.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.09.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "sinc.h"
#include "../../../misc/logger.h"





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
sinc_init (sinc_t *sinc, sinc_side side, opool_t *pool)
{
	int error;
	
	if (unlikely(sinc == NULL))
		LOG_ERROR_AND_RETURN(-1, "null sinc_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)sinc, "dsp-filter-sinc", (cobject_destroy_func)sinc_destroy, pool))))
		LOG_ERROR_AND_RETURN(-1, "failed to dsp_init, %d", error);
	
	sinc->side = side;
	
	sinc->dsp.__feed = (__dsp_feed_func)sinc_feed;
	sinc->dsp.__reset = (__dsp_reset_func)sinc_reset;
	
	return 0;
}

/**
 *
 *
 */
int
sinc_destroy (sinc_t *sinc)
{
	
	return 0;
}





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
sinc_feed (sinc_t *sinc, uint32_t *size, double *data)
{
	
	return 0;
}

/**
 *
 *
 */
int
sinc_reset (sinc_t *sinc)
{
	
	return 0;
}





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
inline sinc_t*
sinc_retain (sinc_t *sinc)
{
	
	return NULL;
}

/**
 *
 *
 */
inline void
sinc_release (sinc_t *sinc)
{
	
}
