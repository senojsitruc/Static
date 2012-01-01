/*
 *  eventhandler.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.16.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  An event dispatcher system. Use this to send events to any and all interested parties.
 *
 */

#ifndef __EVENT_HANDLER_H__
#define __EVENT_HANDLER_H__

#include <stdint.h>
#include "../mem/cobject.h"
#include "../mem/opool.h"

struct event;

//
// eventhandler
//
struct eventhandler
{
	cobject_t cobject;								// parent class
	
	void *context;										// owner's context object
	
	void *sender;											// limit events to only this sender
	uint64_t type;										// limit events to only this type
	
	int (*event_fp)(struct eventhandler*, struct event*);
};
typedef struct eventhandler eventhandler_t;

//
// event_fp_func
//
typedef int (*event_fp_func)(eventhandler_t*, struct event*);





/**
 * eventhandler, context, sender, type, callback
 */
int eventhandler_init (eventhandler_t*, void*, void*, uint64_t, event_fp_func, opool_t*);

/**
 *
 */
int eventhandler_destroy (eventhandler_t*);





/**
 *
 */
int eventhandler_handle (eventhandler_t*, struct event*);





/**
 *
 */
eventhandler_t* eventhandler_retain (eventhandler_t*);

/**
 *
 */
void eventhandler_release (eventhandler_t*);

#endif /* __EVENT_HANDLER_H__ */
