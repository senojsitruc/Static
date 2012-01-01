/*
 *  dataobject.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "dataobject.h"
#include "../misc/logger.h"





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
dataobject_init (dataobject_t *dataobject, opool_t *pool)
{
	int error;
	
	if (unlikely(dataobject == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dataobject_t");
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)dataobject, "dataobject", (cobject_destroy_func)dataobject_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, %d", error);
	
	dataobject->size = 0;
	
	return 0;
}

/**
 *
 *
 */
int
dataobject_destroy (dataobject_t *dataobject)
{
	int error;
	
	if (unlikely(dataobject == NULL))
		LOG_ERROR_AND_RETURN(-1, "null dataobject_t");
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)dataobject))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
inline dataobject_t*
dataobject_retain (dataobject_t *dataobject)
{
	if (unlikely(dataobject == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null dataobject_t");
	
	return (dataobject_t*)cobject_retain((cobject_t*)dataobject);
}

/**
 *
 *
 */
inline void
dataobject_release (dataobject_t *dataobject)
{
	if (unlikely(dataobject == NULL))
		LOG_ERROR_AND_RETURN(, "null dataobject_t");
	
	cobject_release((cobject_t*)dataobject);
}
