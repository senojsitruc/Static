/*
 *  eventmngr.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.16.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __EVENT_MNGR_H__
#define __EVENT_MNGR_H__

#include <stdint.h>
#include "eventhandler.h"
#include "../mem/cobject.h"
#include "../thread/mutex.h"
#include "../thread/thread.h"
#include "../mem/opool.h"
#include "../struct/flist.h"

struct event;
struct eventhandler;

//
// eventmngr
//
struct eventmngr
{
	cobject_t cobject;								// parent class
	
	int stop;													// stop thread
	mutex_t mutex;										// mutex
	thread1_t thread;									// thread
	flist_t events;										// event queue
	struct eventhandler *handlers[50];// event handlers
	uint32_t event_count;							// number of events processed
	uint32_t handler_count;						// number of event handlers in handlers[]
};
typedef struct eventmngr eventmngr_t;





/**
 * eventmngr, event queue size, pool
 */
int eventmngr_init (eventmngr_t*, uint32_t, opool_t*);

/**
 *
 */
int eventmngr_destroy (eventmngr_t*);





/**
 *
 */
int eventmngr_handler_add (eventmngr_t*, struct eventhandler*);

/**
 * eventmngr, handler, context, sender, type, callback
 */
int eventmngr_handler_add2 (eventmngr_t*, struct eventhandler**, void*, void*, uint64_t, int (*)(struct eventhandler*,struct event*));

/**
 *
 */
int eventmngr_handler_del (eventmngr_t*, struct eventhandler*);

/**
 *
 */
int eventmngr_event_post (eventmngr_t*, struct event*);





/**
 *
 */
eventmngr_t* eventmngr_retain (eventmngr_t*);

/**
 *
 */
void eventmngr_release (eventmngr_t*);

#endif /* __EVENT_MNGR_H__ */
