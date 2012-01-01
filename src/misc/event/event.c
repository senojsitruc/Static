/*
 *  event.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.16.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "event.h"
#include "../logger.h"





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
event_init (event_t *event, void *sender, uint64_t type, char *name, cobject_destroy_func destroy, opool_t *pool)
{
	int error;
	
	if (unlikely(event == NULL))
		LOG_ERROR_AND_RETURN(-1, "null event_t");
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)event, name, destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, %d", error);
	
	event->sender = sender;
	event->type = type;
	
	return 0;
}

/**
 *
 *
 */
int
event_destroy (event_t *event)
{
	int error;
	
	if (unlikely(event == NULL))
		LOG_ERROR_AND_RETURN(-1, "null event_t");
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)event))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline event_t*
event_retain (event_t *event)
{
	if (unlikely(event == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null event_t");
	
	return (event_t*)cobject_retain((cobject_t*)event);
}

/**
 *
 *
 */
inline void
event_release (event_t *event)
{
	if (unlikely(event == NULL))
		LOG_ERROR_AND_RETURN(, "null event_t");
	
	cobject_release((cobject_t*)event);
}
