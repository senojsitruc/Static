/*
 *  event.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.16.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __EVENT_H__
#define __EVENT_H__

#include "../mem/cobject.h"
#include "../mem/opool.h"

#define EVENT_CATEGORY_MASK 0xFFFF0000
#define EVENT_TYPE_MASK 0x0000FFFF

//
// event_category
//
typedef enum
{
	EVENT_DEVICE = 0x00010000									// device events
} event_category;

//
// event_type
//
typedef enum
{
	EVENT_DEVICE_FREQ = EVENT_DEVICE|0x0001		// device frequency
} event_type;

//
// event
//
struct event
{
	cobject_t cobject;								// parent class
	
	void *sender;											// sender of the event
	uint64_t type;										// event type
};
typedef struct event event_t;





/**
 * event, sender, type, name, destroy func, pool
 */
int event_init (event_t*, void*, uint64_t, char*, cobject_destroy_func, opool_t*);

/**
 *
 */
int event_destroy (event_t*);





/**
 *
 */
event_t* event_retain (event_t*);

/**
 *
 */
void event_release (event_t*);

#endif /* __EVENT_H__ */
