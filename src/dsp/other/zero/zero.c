/*
 *  zero.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.28.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "zero.h"
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
dspzero_init (dspzero_t *dspzero, dspzero_mode mode, uint32_t offset, uint32_t width, opool_t *pool)
{
	int error;
	
	if (unlikely(dspzero == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspzero_t");
	
	if (unlikely(width == 0 || width > sizeof(double) * 4096))
		LOG_ERROR_AND_RETURN(-2, "invalid data_size (%u)", width);
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)dspzero, "dsp-other-dspzero", (cobject_destroy_func)dspzero_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	dspzero->mode = mode;
	dspzero->width = width;
	dspzero->offset = offset;
	
	dspzero->dsp.__feed = (__dsp_feed_func)dspzero_feed;
	dspzero->dsp.__reset = (__dsp_reset_func)dspzero_reset;
	
	return 0;
}

/**
 *
 *
 */
int
dspzero_destroy (dspzero_t *dspzero)
{
	int error;
	
	if (unlikely(dspzero == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspzero_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)dspzero))))
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
dspzero_feed (dspzero_t *dspzero, uint32_t *size, uint8_t *data)
{
	size_t width, offset, _size;
	
	if (unlikely(dspzero == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspzero_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	width = dspzero->width;
	offset = dspzero->offset;
	_size = *size;
	
	// if the offset is greater than the data size and we're in "exclusive" mode, then indicate that
	// we should zero all available data
	if (offset > _size && DSPZERO_MODE_EXCLUSIVE == dspzero->mode) {
		offset = 0;
		width = _size;
	}
	
	// if the offset plus the width exceeds the available data, resize the width to exactly match the
	// available data.
	else if (offset + width > _size)
		width = _size - offset;
	
	// inclusive - zero out the indicated range
	if (DSPZERO_MODE_INCLUSIVE == dspzero->mode)
		memset(data+offset, 0, width);
	
	// exclusive - zero out everything on either side of the indicated range
	else if (DSPZERO_MODE_EXCLUSIVE == dspzero->mode) {
		memset(data, 0, offset);
		
		if (_size > offset + width)
			memset(data+offset+width, 0, _size-offset-width);
	}
	
	return 0;
}

/**
 *
 *
 */
int
dspzero_reset (dspzero_t *dspzero)
{
	if (unlikely(dspzero == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspzero_t");
	
	return 0;
}	





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline dspzero_t*
dspzero_retain (dspzero_t *dspzero)
{
	if (unlikely(dspzero == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null dspzero_t");
	
	return (dspzero_t*)dsp_retain((dsp_t*)dspzero);
}

/**
 *
 *
 */
inline void
dspzero_release (dspzero_t *dspzero)
{
	if (unlikely(dspzero == NULL))
		LOG_ERROR_AND_RETURN(, "null dspzero_t");
	
	dsp_release((dsp_t*)dspzero);
}

