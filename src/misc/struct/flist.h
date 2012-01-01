/*
 *  flist.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  A fixed-size, lock-free, wait-free fifo queue. The push() and pop() functions use atomic
 *  operations and _ought_ to hold up under the strain of multiple readers and writers, but I have
 *  not tested it.
 *
 */

#ifndef __FLIST_H__
#define __FLIST_H__

#include "../mem/cobject.h"
#include "../mem/opool.h"
#include <stdint.h>

//
// flist
//
struct flist
{
	cobject_t cobject;								// parent class
	
	int64_t size;											// list size
	int64_t volatile count;						// item count
	int64_t volatile head;						// head location in list
	int64_t volatile tail;						// tail location in list
	
	cobject_t **list;									// list
};
typedef struct flist flist_t;





/**
 * flist, size, pool
 */
int flist_init (flist_t*, uint32_t, opool_t*);

/**
 *
 */
int flist_destroy (flist_t*);





/**
 *
 */
int flist_push (flist_t*, cobject_t*);

/**
 *
 */
int flist_pop (flist_t*, cobject_t**);

/**
 *
 */
int flist_clear (flist_t*);





/**
 *
 */
flist_t* flist_retain (flist_t*);

/**
 *
 */
void flist_release (flist_t*);

#endif /* __FLIST_H__ */
