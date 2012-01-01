/*
 *  demod.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.29.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "demod.h"
#include "../../misc/logger.h"





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
demod_init (demod_t *demod, char *name, cobject_destroy_func destroy, opool_t *pool)
{
	int error;
	
	if (unlikely(demod == NULL))
		LOG_ERROR_AND_RETURN(-1, "null demod_t");
	
	if (unlikely(0 != (error = dsp_init((dsp_t*)demod, name, destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to dsp_init, %d", error);
	
	demod->dsp.__feed = (__dsp_feed_func)demod_feed;
	demod->dsp.__reset = (__dsp_reset_func)demod_reset;
	
	return 0;
}

/**
 *
 *
 */
int
demod_destroy (demod_t *demod)
{
	int error;
	
	if (unlikely(demod == NULL))
		LOG_ERROR_AND_RETURN(-1, "null demod_t");
	
	if (unlikely(0 != (error = dsp_destroy((dsp_t*)demod))))
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
demod_feed (demod_t *demod, uint32_t *size, double *data)
{
	if (unlikely(demod == NULL))
		LOG_ERROR_AND_RETURN(-1, "null demod_t");
	
	if (unlikely(demod->__feed == NULL))
		LOG_ERROR_AND_RETURN(-2, "no '__feed' callback");
	
	return (*demod->__feed)((dsp_t*)demod, size, data);
}

/**
 *
 *
 */
int
demod_reset (demod_t *demod)
{
	if (unlikely(demod == NULL))
		LOG_ERROR_AND_RETURN(-1, "null demod_t");
	
	if (unlikely(demod->__reset == NULL))
		LOG_ERROR_AND_RETURN(-2, "no '__reset' callback");
	
	return (*demod->__reset)((dsp_t*)demod);
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline demod_t*
demod_retain (demod_t *demod)
{
	if (unlikely(demod == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null demod_t");
	
	return (demod_t*)dsp_retain((dsp_t*)demod);
}

/**
 *
 *
 */
inline void
demod_release (demod_t *demod)
{
	if (unlikely(demod == NULL))
		LOG_ERROR_AND_RETURN(, "null demod_t");
	
	dsp_release((dsp_t*)demod);
}
