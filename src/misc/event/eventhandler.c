/*
 *  eventhandler.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.16.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "eventhandler.h"
#include "event.h"
#include "../logger.h"





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
eventhandler_init (eventhandler_t *eventhandler, void *context, void *sender, uint64_t type, event_fp_func callback, opool_t *pool)
{
	int error;
	
	if (unlikely(eventhandler == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventhandler_t");
	
	if (unlikely(callback == NULL))
		LOG_ERROR_AND_RETURN(-2, "null event_fp_func");
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)eventhandler, "eventhandler", (cobject_destroy_func)eventhandler_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, %d", error);
	
	eventhandler->context = context;
	eventhandler->sender = sender;
	eventhandler->type = type;
	eventhandler->event_fp = callback;
	
	return 0;
}

/**
 *
 *
 */
int
eventhandler_destroy (eventhandler_t *eventhandler)
{
	int error;
	
	if (unlikely(eventhandler == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventhandler_t");
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)eventhandler))))
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
eventhandler_handle (eventhandler_t *eventhandler, event_t *event)
{
	if (unlikely(eventhandler == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventhandler_t");
	
	if (unlikely(event == NULL))
		LOG_ERROR_AND_RETURN(-2, "null event_t");
	
	if (unlikely(eventhandler->event_fp == NULL))
		LOG_ERROR_AND_RETURN(-3, "null event_fp");
	
	return (*eventhandler->event_fp)(eventhandler, event);
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline eventhandler_t*
eventhandler_retain (eventhandler_t *eventhandler)
{
	if (unlikely(eventhandler == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null eventhandler_t");
	
	return (eventhandler_t*)cobject_retain((cobject_t*)eventhandler);
}

/**
 *
 *
 */
void
eventhandler_release (eventhandler_t *eventhandler)
{
	if (unlikely(eventhandler == NULL))
		LOG_ERROR_AND_RETURN(, "null eventhandler_t");
	
	cobject_release((cobject_t*)eventhandler);
}
