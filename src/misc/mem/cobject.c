/*
 *  cobject.c
 *  Chatter
 *
 *  Created by Curtis Jones on 2009.10.14.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "cobject.h"
#include "opool.h"
#include "../logger.h"
#include <string.h>

#pragma mark -
#pragma mark structors

/**
 *
 *
 */
inline int
cobject_init (cobject_t *cobject, char *name, cobject_destroy_func destroy_func, opool_t *pool)
{
	int error;
	size_t name_l;
	
	if (unlikely(cobject == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cobject_t");
	
	if (unlikely(name == NULL || (name_l = strlen(name)) == 0))
		LOG_ERROR_AND_RETURN(-2, "null or zero-length name");
	
	if (unlikely(destroy_func == NULL))
		LOG_ERROR_AND_RETURN(-3, "null cobject_destroy_func");
	
	if (name_l >= sizeof(cobject->name))
		name_l = sizeof(cobject->name) - 1;
	
	memcpy(cobject->name, name, name_l);
	
	if (unlikely(0 != (error = memlock_init(&cobject->memlock, (memlock_destroy_func)destroy_func, cobject))))
		LOG_ERROR_AND_RETURN(-101, "[%s] failed to memlock_init, 0x%08X [%d]", cobject->name, error, error);
	
	if (pool != NULL)
		cobject->pool = opool_retain(pool);
	else
		cobject->pool = NULL;
	
	return 0;
}

/**
 *
 *
 */
inline int
cobject_destroy (cobject_t *cobject)
{
	int error;
	opool_t *pool;
	
	if (unlikely(cobject == NULL))
		LOG_ERROR_AND_RETURN(-1, "null cobject_t");
	
	if (unlikely(0 != (error = memlock_destroy(&cobject->memlock))))
		LOG_ERROR_AND_RETURN(-101, "[%s] failed to memlock_destroy, 0x%08X [%d]", cobject->name, error, error);
	
	if (cobject->pool != NULL) {
		pool = cobject->pool;
		cobject->pool = NULL;
		opool_push(pool, cobject);
		opool_release(pool);
	}
	else
		LOG3("[%s] ||||||||||||||||||||||||||| [2]", cobject->name);
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline cobject_t*
cobject_retain (cobject_t *cobject)
{
	if (unlikely(cobject == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null cobject_t");
	
	memlock_retain(&cobject->memlock);
	
	return cobject;
}

/**
 *
 */
inline void
cobject_release (cobject_t *cobject)
{
	if (unlikely(cobject == NULL))
		LOG_ERROR_AND_RETURN(, "null cobject_t");
	
	memlock_release(&cobject->memlock);
}
