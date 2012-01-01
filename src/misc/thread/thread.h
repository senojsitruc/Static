/*
 *  thread.h
 *  Static
 *
 *  Created by Curtis Jones on 2008.10.20.
 *  Copyright 2008 Curtis Jones. All rights reserved.
 *
 */

#ifndef __THREAD_H__
#define __THREAD_H__

#include <stdint.h>
#include <pthread.h>
#include "../mem/cobject.h"
#include "../mem/opool.h"

//
// thread_state
//
typedef enum
{
	THREAD_STATE_ERROR   = -1,
	THREAD_STATE_STARTED = (1<<0),
	THREAD_STATE_STOPPED = (1<<1),
	THREAD_STATE_STOPPING = (1<<2)
} thread_state;

//
// thread
//
struct thread
{
	cobject_t cobject;								// super class
	
	pthread_t pthread;								// posix thread
	thread_state volatile state;			// state
	
	void *(*start_func)(void*);				// start func
	void *arg;												// arg
};
typedef struct thread thread1_t;

//
// thread_start_func
//
typedef void * (*thread_start_func) (void *);

/**
 * thread, name, start function pointer, context, pool
 */
int thread_init (thread1_t*, char*, thread_start_func, void*restrict, opool_t*);

/**
 *
 */
int thread_destroy (thread1_t*);

/**
 *
 */
int thread_start (thread1_t*);

/**
 *
 */
int thread_stop (thread1_t*);

/**
 *
 */
thread_state thread_getstate (thread1_t*);

/**
 *
 */
int thread_signal (thread1_t*, int);

/**
 *
 */
thread1_t* thread_retain (thread1_t*);

/**
 *
 */
void thread_release (thread1_t*);

#endif /* __THREAD_H__ */
