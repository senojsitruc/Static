/*
 *  opool.h
 *  Chatter
 *
 *  Created by Curtis Jones on 2009.03.26.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __O_POOL_H__
#define __O_POOL_H__

#include "cobject.h"

//
// opool
//
struct opool
{
	cobject_t cobject;								// super class
	
	char name[100];										// object pool name
	uint64_t osize;										// object size (in bytes)
	uint64_t bsize;										// block size (in objects)
	uint64_t bmax;										// max blocks
	
	int64_t inuse;										// number of objects in use
	int64_t high;											// high "water mark" of "inuse"
	int64_t bcount;										// current number of allocated blocks
	int64_t ocount;										// current number of allocated objects
	
	int64_t size;											// fixed capacity of list
	int64_t volatile count;						// number of objects in list
	int64_t volatile head;						// head location in list
	int64_t volatile tail;						// tail location in list
	
	void **list;											// list
	
	int (*init)(struct opool*,void*);
};
typedef struct opool opool_t;

//
// opool_object_init_func
//
typedef int (*opool_object_init_func) (opool_t*, void*);





/**
 * opool, object size, block size (in objects), max blocks, pool name
 */
int opool_init (opool_t*, uint64_t, uint64_t, uint64_t, char*);

/**
 * opool, object size, block size (in objects), max blocks, pool 
 * name, object init function pointer
 */
int opool_init2 (opool_t*, uint64_t, uint64_t, uint64_t, char*, opool_object_init_func);

/**
 * opool
 */
int opool_destroy (opool_t*);





/**
 *
 */
int opool_push (opool_t*, void*);

/**
 *
 */
int opool_pop (opool_t*, void**);





/**
 *
 */
opool_t* opool_retain (opool_t*);

/**
 *
 */
void opool_release (opool_t*);

#endif /* __O_POOL_H__ */
