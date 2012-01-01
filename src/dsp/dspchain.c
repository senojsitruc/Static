/*
 *  dspchain.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "dspchain.h"
#include "../misc/logger.h"
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
dspchain_init (dspchain_t *dspchain, uint32_t size, opool_t *pool)
{
	int error;
	
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspchain_t");
	
	if (unlikely(size == 0 || size > 1000))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)dspchain, "dspchain", (cobject_destroy_func)dspchain_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, %d", error);
	
	if (unlikely(NULL == (dspchain->dsp_list = (dsp_t**)malloc(sizeof(dsp_t*) * size))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc, %s", strerror(errno));
	
	dspchain->dsp_size = size;
	memset(dspchain->dsp_list, 0, sizeof(dsp_t*) * size);
	
	return 0;
}

/**
 *
 *
 */
int
dspchain_destroy (dspchain_t *dspchain)
{
	int error;
	dsp_t *dsp = NULL;
	
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspchain_t");
	
	for (uint32_t i = 0; i < dspchain->dsp_size; ++i) {
		if (dspchain->dsp_list[i] != NULL) {
			dsp = dspchain->dsp_list[i];
			dspchain->dsp_list[i] = NULL;
			dspchain->dsp_count -= 1;
			dsp_release(dsp);
		}
	}
	
	free(dspchain->dsp_list);
	dspchain->dsp_list = NULL;
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)dspchain))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
dspchain_add (dspchain_t *dspchain, dsp_t *dsp, int32_t ndx)
{
	uint32_t i;
	
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspchain_t");
	
	if (unlikely(dsp == NULL))
		LOG_ERROR_AND_RETURN(-2, "null dsp_t");
	
	if (unlikely(ndx != DSPCHAIN_NEXT_POS && ndx >= (int32_t)dspchain->dsp_size))
		LOG_ERROR_AND_RETURN(-3, "invalid index (%d)", ndx);
	
	if (dspchain->dsp_count == dspchain->dsp_size)
		LOG_ERROR_AND_RETURN(-101, "[1] no space left in dsp list");
	
	if (ndx == DSPCHAIN_NEXT_POS) {
		for (i = 0; i < dspchain->dsp_size; ++i) {
			if (dspchain->dsp_list[i] == NULL)
				break;
		}
		
		if (i == dspchain->dsp_size)
			LOG_ERROR_AND_RETURN(-102, "[2] no space left in dsp list");
		
		ndx = (int32_t)i;
	}
	
	if (dspchain->dsp_list[ndx] != NULL)
		LOG_ERROR_AND_RETURN(-103, "this slot is already spoken for");
	
	dspchain->dsp_list[ndx] = dsp_retain(dsp);
	dspchain->dsp_count += 1;
	
	LOG3("added '%s' at index %d", dsp->cobject.name, ndx);
	
	return 0;
}

/**
 *
 *
 */
int
dspchain_del (dspchain_t *dspchain, uint32_t ndx)
{
	dsp_t *dsp;
	
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspchain_t");
	
	if (unlikely(ndx >= dspchain->dsp_size))
		LOG_ERROR_AND_RETURN(-2, "invalid index (%u)", ndx);
	
	dsp = dspchain->dsp_list[ndx];
	
	if (dsp != NULL) {
		dspchain->dsp_list[ndx] = NULL;
		dspchain->dsp_count--;
		dsp_release(dsp);
	}
	
	return 0;
}

/**
 *
 *
 */
int
dspchain_get (dspchain_t *dspchain, uint32_t ndx, dsp_t **dsp)
{
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspchain_t");
	
	if (unlikely(ndx >= dspchain->dsp_size))
		LOG_ERROR_AND_RETURN(-2, "invalid index (%u)", ndx);
	
	if (unlikely(dsp == NULL))
		LOG_ERROR_AND_RETURN(-3, "null dsp_t");
	
	*dsp = dspchain->dsp_list[ndx];
	
	return 0;
}

/**
 *
 *
 */
int
dspchain_cnt (dspchain_t *dspchain, uint32_t *cnt)
{
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspchain_t");
	
	if (unlikely(cnt == NULL))
		LOG_ERROR_AND_RETURN(-2, "null count return value");
	
	*cnt = dspchain->dsp_count;
	
	return 0;
}

/**
 *
 *
 */
int
dspchain_run (dspchain_t *dspchain, uint32_t *size, void *data)
{
	int error;
	dsp_t *dsp;
	
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspchain_t");
	
	for (uint32_t i=0, f=0; i < dspchain->dsp_size && f < dspchain->dsp_count; ++i) {
		if (NULL == (dsp = dspchain->dsp_list[i]))
			continue;
		
		// a non-zero reply from a dsp_t indicates an error
		if (unlikely(0 != (error = dsp_feed(dsp, size, data))))
			LOG_ERROR_AND_RETURN(-101, "failed to dsp_feed, %d", error);
		
		// a filter can set the size to zero if it is not yet "full" in some sense, and processing
		// subsequent dsp processing is unnecessary.
		if (*size == 0)
			break;
		
		f++;
	}
	
	return 0;
}

/**
 *
 *
 */
int
dspchain_reset (dspchain_t *dspchain)
{
	int error;
	dsp_t *dsp;
	
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dspchain_t");
	
	for (uint32_t i=0, f=0; i < dspchain->dsp_size && f < dspchain->dsp_count; ++i) {
		if (NULL == (dsp = dspchain->dsp_list[i]))
			continue;
		
		if (unlikely(0 != (error = dsp_reset(dsp))))
			LOG3("failed to dsp_reset, %d", error);
		
		f++;
	}
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline dspchain_t*
dspchain_retain (dspchain_t *dspchain)
{
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null dspchain_t");
	
	return (dspchain_t*)cobject_retain((cobject_t*)dspchain);
}

/**
 *
 *
 */
inline void
dspchain_release (dspchain_t *dspchain)
{
	if (unlikely(dspchain == NULL))
		LOG_ERROR_AND_RETURN(, "null dspchain_t");
	
	cobject_release((cobject_t*)dspchain);
}
