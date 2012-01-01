/*
 *  trim.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.20.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "trim.h"
#include "../../../misc/atomic.h"
#include "../../../misc/logger.h"
#include <errno.h>
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
dsptrim_init (dsptrim_t *dsptrim, uint32_t width, uint32_t offset, opool_t *pool)
{
	int error;
	
	if (unlikely(dsptrim == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dsptrim_t");
	
	if (unlikely(width == 0 || width > sizeof(double) * 4096))
		LOG_ERROR_AND_RETURN(-2, "invalid data_size (%u)", width);
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)dsptrim, "dsp-other-dsptrim", (cobject_destroy_func)dsptrim_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	dsptrim->width = width;
	dsptrim->offset = offset;
	
	dsptrim->dsp.__feed = (__dsp_feed_func)dsptrim_feed;
	dsptrim->dsp.__reset = (__dsp_reset_func)dsptrim_reset;
	
	return 0;
}

/**
 *
 *
 */
int
dsptrim_destroy (dsptrim_t *dsptrim)
{
	int error;
	
	if (unlikely(dsptrim == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dsptrim_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)dsptrim))))
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
dsptrim_feed (dsptrim_t *dsptrim, uint32_t *size, uint8_t *data)
{
	size_t width, offset, _size;
	
	if (unlikely(dsptrim == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dsptrim_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	width = dsptrim->width;
	offset = dsptrim->offset;
	_size = *size;
	
	if (unlikely(offset >= _size))
		_size = 0;
	
	else {
		if (offset != 0) {
			memmove(data, data+offset, _size-offset);
			_size -= offset;
		}
		
		if (width < _size)
			_size = width;
	}
	
	*size = (uint32_t)_size;
	
	return 0;
}

/**
 *
 *
 */
int
dsptrim_reset (dsptrim_t *dsptrim)
{
	if (unlikely(dsptrim == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dsptrim_t");
	
	return 0;
}	





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline dsptrim_t*
dsptrim_retain (dsptrim_t *dsptrim)
{
	if (unlikely(dsptrim == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null dsptrim_t");
	
	return (dsptrim_t*)dsp_retain((dsp_t*)dsptrim);
}

/**
 *
 *
 */
inline void
dsptrim_release (dsptrim_t *dsptrim)
{
	if (unlikely(dsptrim == NULL))
		LOG_ERROR_AND_RETURN(, "null dsptrim_t");
	
	dsp_release((dsp_t*)dsptrim);
}

