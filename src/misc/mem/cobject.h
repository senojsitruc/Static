/*
 *  cobject.h
 *  Chatter
 *
 *  Created by Curtis Jones on 2009.10.14.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __C_OBJECT_H__
#define __C_OBJECT_H__

#include "memlock.h"

struct opool;

//
// cobject
//
struct cobject
{
	memlock_t memlock;								// reference counting mechanism
	struct opool *pool;								// object pool
	char name[100];										// instance name
};
typedef struct cobject cobject_t;

//
// cobject_destroy_func
//
typedef int (*cobject_destroy_func)(cobject_t*);

/**
 * cobject, name, destroy function pointer, object pool (optional)
 */
int cobject_init (cobject_t*, char*, cobject_destroy_func, struct opool*);

/**
 *
 */
int cobject_destroy (cobject_t*);

/**
 *
 */
cobject_t* cobject_retain (cobject_t*);

/**
 *
 */
void cobject_release (cobject_t*);

#endif /* __C_OBJECT_H__ */
