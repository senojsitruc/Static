/*
 *  eventmngr.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.16.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "eventmngr.h"
#include "event.h"
#include "eventhandler.h"
#include "../logger.h"
#include "../../core/core.h"
#include <unistd.h>

struct eventhandler;
void* eventmngr_thread (eventmngr_t*);





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
eventmngr_init (eventmngr_t *eventmngr, uint32_t size, opool_t *pool)
{
	int error;
	
	if (unlikely(eventmngr == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventmngr_t");
	
	if (unlikely(size == 0))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)eventmngr, "eventmngr", (cobject_destroy_func)eventmngr_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, %d", error);
	
	if (unlikely(0 != (error = flist_init(&eventmngr->events, size, pool))))
		LOG_ERROR_AND_RETURN(-102, "failed to flist_init, %d", error);
	
	if (unlikely(0 != (error = mutex_init(&eventmngr->mutex, LOCK_TYPE_SPIN))))
		LOG_ERROR_AND_RETURN(-103, "failed to mutex_init, %d", error);
	
	if (unlikely(0 != (error = thread_init(&eventmngr->thread, "eventmngr", (thread_start_func)eventmngr_thread, eventmngr, NULL))))
		LOG_ERROR_AND_RETURN(-104, "failed to thread_init, %d", error);
	
	if (unlikely(0 != (error = thread_start(&eventmngr->thread))))
		LOG_ERROR_AND_RETURN(-105, "failed to thread_start, %d", error);
	
	eventmngr->stop = 0;
	
	return 0;
}

/**
 *
 *
 */
int
eventmngr_destroy (eventmngr_t *eventmngr)
{
	int error;
	
	if (unlikely(eventmngr == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventmngr_t");
	
	if (unlikely(0 != (error = thread_destroy(&eventmngr->thread))))
		LOG_ERROR_AND_RETURN(-101, "failed to thread_destroy, %d", error);
	
	if (unlikely(0 != (error = mutex_destroy(&eventmngr->mutex))))
		LOG_ERROR_AND_RETURN(-102, "failed to mutex_destroy, %d", error);
	
	if (unlikely(0 != (error = flist_destroy(&eventmngr->events))))
		LOG_ERROR_AND_RETURN(-103, "failed to flist_destroy, %d", error);
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)eventmngr))))
		LOG_ERROR_AND_RETURN(-104, "failed to cobject_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
eventmngr_handler_add (eventmngr_t *eventmngr, eventhandler_t *eventhandler)
{
	int i, count;
	
	if (unlikely(eventmngr == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventmngr_t");
	
	if (unlikely(eventhandler == NULL))
		LOG_ERROR_AND_RETURN(-2, "null eventhandler_t");
	
	count = sizeof(eventmngr->handlers) / sizeof(eventhandler_t*);
	
	mutex_lock(&eventmngr->mutex);
	
	for (i = 0; i < count; ++i) {
		if (eventmngr->handlers[i] == NULL) {
			eventmngr->handlers[i] = eventhandler_retain(eventhandler);
			eventmngr->handler_count++;
			LOG3("added handler at index %d for type 0x%08llX", i, eventhandler->type);
			break;
		}
	}
	
	mutex_unlock(&eventmngr->mutex);
	
	return 0;
}

/**
 *
 *
 */
int
eventmngr_handler_add2 (eventmngr_t *eventmngr, struct eventhandler **eventhandler_ptr, void *context, void *sender, uint64_t type, int (*callback)(struct eventhandler*,struct event*))
{
	int error;
	struct eventhandler *eventhandler = NULL;
	
	if (unlikely(eventmngr == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventmngr_t");
	
	if (unlikely(0 != (error = core_misc_eventhandler(&eventhandler, context, sender, type, callback))))
		LOG_ERROR_AND_RETURN(-101, "failed to core_misc_eventhandler, %d", error);
	
	if (eventhandler_ptr != NULL)
		*eventhandler_ptr = eventhandler;
	
	return eventmngr_handler_add(eventmngr, eventhandler);
}

/**
 *
 *
 */
int
eventmngr_handler_del (eventmngr_t *eventmngr, eventhandler_t *eventhandler)
{
	int i, count;
	
	if (unlikely(eventmngr == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventmngr_t");
	
	if (unlikely(eventhandler == NULL))
		LOG_ERROR_AND_RETURN(-2, "null eventhandler_t");
	
	count = sizeof(eventmngr->handlers) / sizeof(eventhandler_t*);
	
	mutex_lock(&eventmngr->mutex);
	
	for (i = 0; i < count; ++i) {
		if (eventmngr->handlers[i] == eventhandler) {
			eventmngr->handlers[i] = NULL;
			eventmngr->handler_count--;
			eventhandler_release(eventhandler);
			break;
		}
	}
	
	mutex_unlock(&eventmngr->mutex);
	
	return 0;
}

/**
 *
 *
 */
int
eventmngr_event_post (eventmngr_t *eventmngr, event_t *event)
{
	int error;
	
	if (unlikely(eventmngr == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventmngr_t");
	
	if (unlikely(event == NULL))
		LOG_ERROR_AND_RETURN(-2, "null event_t");
	
	mutex_lock(&eventmngr->mutex);
	
	if (unlikely(0 != (error = flist_push(&eventmngr->events, (cobject_t*)event))))
		LOG_ERROR_AND_RETURN(-101, "failed to flist_push, %d", error);
	
	eventmngr->event_count++;
	
	mutex_unlock(&eventmngr->mutex);
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline eventmngr_t*
eventmngr_retain (eventmngr_t *eventmngr)
{
	if (unlikely(eventmngr == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null eventmngr_t");
	
	return (eventmngr_t*)cobject_retain((cobject_t*)eventmngr);
}

/**
 *
 *
 */
inline void
eventmngr_release (eventmngr_t *eventmngr)
{
	if (unlikely(eventmngr == NULL))
		LOG_ERROR_AND_RETURN(, "null eventmngr_t");
	
	cobject_release((cobject_t*)eventmngr);
}




#pragma mark -
#pragma mark thread stuff

/**
 *
 *
 */
void*
eventmngr_thread (eventmngr_t *eventmngr)
{
	int error;
	uint32_t handler_count, handler_size;
	event_t *event = NULL;
	eventhandler_t *handler;
	
	handler_size = sizeof(eventmngr->handlers) / sizeof(eventhandler_t*);
	
	if (unlikely(eventmngr == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null eventmngr_t");
	
	while (!eventmngr->stop) {
		if (unlikely(0 != (error = flist_pop(&eventmngr->events, (cobject_t**)&event)))) {
			LOG_ERROR_AND_RETURN(NULL, " failed to flist_pop, %d", error);
		}
		else if (event == NULL) {
			usleep(10000);
			continue;
		}
		
		handler_count = 0;
		
		for (uint32_t i = 0; i < handler_size && handler_count < eventmngr->handler_count; ++i) {
			if ((handler = eventmngr->handlers[i]) != NULL) {
				// if 1) the handler type does not equal the event type and 2) the category portion of the
				// handler type does not equal the category portion of the event type, then skip this
				// handler for this event.
				if (handler->type != event->type && (handler->type&EVENT_CATEGORY_MASK) != (event->type&EVENT_CATEGORY_MASK))
					continue;
				
				// give the event to the handler
				if (unlikely(0 != (error = eventhandler_handle(handler, event))))
					LOG3("failed to eventhandler_handle, %d", error);
			}
			
			handler_count++;
		}
		
		event_release(event);
	}
	
	return NULL;
}
