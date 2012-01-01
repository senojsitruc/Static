/*
 *  mutex.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.22.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *  Implementation is based on the paper "The Performance of Spin Lock Alternatives for Shared-
 *  Memory Multiprocessors" (by Thomas E. Anderson) which, granted, is a bit long in the tooth,
 *  but I bet it's a lot faster than doing system calls -- as long as we never expect a lock to be
 *  blocked for a long time and we limit the number of threads to something close to the number of
 *  cores available on the machine.
 *
 */

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <stdint.h>
#include <pthread.h>

//
// locktype
//
enum locktype {
	LOCK_TYPE_SPIN = 1,
	LOCK_TYPE_WAIT
};

//
// mutex
//
struct mutex
{
	int32_t volatile lock;
	int32_t volatile locks[10];
	int32_t volatile last;
	int32_t type;
	
	pthread_mutex_t pmutex;
};
typedef struct mutex mutex_t;

/**
 *
 */
int mutex_init (mutex_t*, enum locktype);

/**
 *
 */
int mutex_destroy (mutex_t*);

/**
 *
 */
int mutex_lock (mutex_t*);

/**
 *
 */
int mutex_unlock (mutex_t*);

/**
 *
 */
int32_t mutex_lock2 (mutex_t*);

/**
 *
 */
int mutex_unlock2 (mutex_t*, int32_t);

#endif /* __MUTEX_H__ */
