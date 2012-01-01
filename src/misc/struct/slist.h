/*
 *  slist.h
 *  Chatter
 *
 *  Created by Curtis Jones on 2009.10.09.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 */

#ifndef __SLIST_H__
#define __SLIST_H__

#include <stdint.h>
#include "../mem/cobject.h"
#include "../mem/opool.h"

//
// slist_item
//
struct slist_item
{
	struct slist_item *next;					// next item
	void *data;												// 
};
typedef struct slist_item slist_item_t;

//
// slist
//
struct slist
{
	slist_item_t * volatile head;			// head of the list
	slist_item_t * volatile tail;			// tail of the list
	uint32_t volatile count;					// item count
	opool_t *pool;										// slist_item_t pool
};
typedef struct slist slist_t;

//
// slist_iter
//
struct slist_iter
{
	slist_item_t *item;								// 
};
typedef struct slist_iter slist_iter_t;

/**
 * list, pool (slist_item_t)
 */
int slist_init (slist_t*, opool_t*);

/**
 *
 */
int slist_destroy (slist_t*);

/**
 *
 */
int slist_clear (slist_t*);

/**
 *
 */
int slist_push (slist_t*, cobject_t*);

/**
 *
 */
int slist_pop (slist_t*, cobject_t**);

/**
 *
 */
int slist_head (slist_t*, cobject_t**);

/**
 *
 */
int slist_tail (slist_t*, cobject_t**);

/**
 *
 */
int slist_peek (slist_t*, cobject_t**, uint32_t);

/**
 *
 */
int slist_iter_init (slist_t*, slist_iter_t*);

/**
 *
 */
int slist_iter_next (slist_iter_t*, cobject_t**);

#endif /* __SLIST_H__ */
