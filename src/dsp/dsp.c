/*
 *  dsp.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "dsp.h"
#include "../misc/logger.h"





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
dsp_init (dsp_t *dsp, char *name, cobject_destroy_func destroy, opool_t *pool)
{
	int error;
	
	if (unlikely(dsp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dsp_t");
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)dsp, name, destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "[%s] failed to cobject_init, %d", name, error);
	
	// enabled by default
	dsp->state = DSP_STATE_ENABLED;
	
	return 0;
}

/**
 *
 *
 */
int
dsp_destroy (dsp_t *dsp)
{
	int error;
	
	if (unlikely(dsp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dsp_t");
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)dsp))))
		LOG_ERROR_AND_RETURN(-101, "[%s] failed to cobject_destroy, %d", dsp->cobject.name, error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
dsp_feed (dsp_t *dsp, uint32_t *size, void *data)
{
	if (unlikely(dsp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dsp_t");
	
	if (unlikely(dsp->state != DSP_STATE_ENABLED))
		return 0;
	
	if (unlikely(dsp->__feed == NULL))
		LOG_ERROR_AND_RETURN(-2, "null '__feed' callback");
	
	return (*dsp->__feed)(dsp, size, data);
}

/**
 *
 *
 */
int
dsp_reset (dsp_t *dsp)
{
	if (unlikely(dsp == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dsp_t");
	
	if (unlikely(dsp->__reset == NULL))
		LOG_ERROR_AND_RETURN(-2, "[%s] null '__reset' callback", dsp->cobject.name);
	
	return (*dsp->__reset)(dsp);
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline dsp_t*
dsp_retain (dsp_t *dsp)
{
	if (unlikely(dsp == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null dsp_t");
	
	return (dsp_t*)cobject_retain((cobject_t*)dsp);
}

/**
 *
 *
 */
inline void
dsp_release (dsp_t *dsp)
{
	if (unlikely(dsp == NULL))
		LOG_ERROR_AND_RETURN(, "null dsp_t");
	
	cobject_release((cobject_t*)dsp);
}
