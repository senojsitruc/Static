/*
 *  shift.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.29.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "shift.h"
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
dspshift_init (dspshift_t *dspshift, dspshift_dir dir, uint32_t size, opool_t *pool)
{
	int error;
	
	if (unlikely(dspshift == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspshift_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)dspshift, "dsp-other-dspshift", (cobject_destroy_func)dspshift_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	dspshift->dir = dir;
	dspshift->size = size;
	
	dspshift->dsp.__feed = (__dsp_feed_func)dspshift_feed;
	dspshift->dsp.__reset = (__dsp_reset_func)dspshift_reset;
	
	return 0;
}

/**
 *
 *
 */
int
dspshift_destroy (dspshift_t *dspshift)
{
	int error;
	
	if (unlikely(dspshift == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspshift_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)dspshift))))
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
dspshift_feed (dspshift_t *dspshift, uint32_t *size, uint8_t *data)
{
	size_t _size;
	
	if (unlikely(dspshift == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspshift_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	_size = *size;
	
	if (dspshift->size < _size)
		memmove(data, data+dspshift->size, _size-dspshift->size);
	
	return 0;
}

/**
 *
 *
 */
int
dspshift_reset (dspshift_t *dspshift)
{
	if (unlikely(dspshift == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspshift_t");
	
	return 0;
}	





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline dspshift_t*
dspshift_retain (dspshift_t *dspshift)
{
	if (unlikely(dspshift == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null dspshift_t");
	
	return (dspshift_t*)dsp_retain((dsp_t*)dspshift);
}

/**
 *
 *
 */
inline void
dspshift_release (dspshift_t *dspshift)
{
	if (unlikely(dspshift == NULL))
		LOG_ERROR_AND_RETURN(, "null dspshift_t");
	
	dsp_release((dsp_t*)dspshift);
}

