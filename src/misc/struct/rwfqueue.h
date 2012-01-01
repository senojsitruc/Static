/*
 *  rwfqueue.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.22.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __RWFQUEUE_H__
#define __RWFQUEUE_H__

#include <stdint.h>
#include <pthread.h>

typedef enum
{
	RWFQUEUE_MODE_BLOCK = 1,
	RWFQUEUE_MODE_NOBLOCK
} rwfqueue_mode;

//
// rwfqueue_item
//
struct rwfqueue_item
{
	void * data;											// 
	uint32_t inuse;										// 
};
typedef struct rwfqueue_item rwfqueue_item_t;

//
// rwfqueue
//
struct rwfqueue
{
	int32_t size;											// 
	int32_t count;										// 
	int32_t stop;											// 
	rwfqueue_mode rmode;							// RWFQUEUE_MODE_BLOCK, NOBLOCK - pop(), peek()
	rwfqueue_mode wmode;							// RWFQUEUE_MODE_BLOCK, NOBLOCK - push()
	struct rwfqueue_item * head;			// 
	struct rwfqueue_item * tail;			// 
	struct rwfqueue_item * last;			// 
	struct rwfqueue_item * data;			// 
	pthread_cond_t cond;							// 
	pthread_mutex_t lock;							// 
};
typedef struct rwfqueue rwfqueue_t;

/* rmode specifies whether read operations (pop, peek, etc) should
 * block if no data is available immediately. wmode specifies whether
 * write operations (push) should block if the queue is full.
 */
int rwfqueue_init (rwfqueue_t *, uint32_t size, rwfqueue_mode rmode, rwfqueue_mode wmode);

/*
 *
 */
void rwfqueue_destroy (rwfqueue_t *);

/*
 *
 */
uint32_t rwfqueue_count (rwfqueue_t *);

/*
 *
 */
int rwfqueue_push (rwfqueue_t *, void *);

/*
 *
 */
void * rwfqueue_pop (rwfqueue_t *);

/*
 *
 */
void * rwfqueue_peek (rwfqueue_t *);

/*
 *
 */
void * rwfqueue_peak2 (rwfqueue_t *, int32_t);

/*
 *
 */
void * rwfqueue_last (rwfqueue_t *);

/*
 *
 */
void * rwfqueue_head (rwfqueue_t *);

#endif /* __RWFQUEUE_H__ */
