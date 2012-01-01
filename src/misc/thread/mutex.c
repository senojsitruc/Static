/*
 *  mutex.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.22.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "mutex.h"
#include "../atomic.h"
#include "../logger.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Only supports spin locking by default.
 *
 */
int
mutex_init (mutex_t *mutex, enum locktype type)
{
	if (unlikely(mutex == NULL))
		LOG_ERROR_AND_RETURN(-1, "null mutex_t");
	
	int error;
	
	mutex->lock = 0;
	mutex->type = type;
//mutex->locks;
	mutex->last = 0;
	
	memset((void*)&mutex->locks, 0, sizeof(mutex->locks));
	
	mutex->locks[0] = 1;
	
	if (type == LOCK_TYPE_WAIT)
		LOG_ERROR_AND_RETURN(-2, "unsupported!");
	
	if (unlikely(0 != (error = pthread_mutex_init(&mutex->pmutex, NULL))))
		LOG_ERROR_AND_RETURN(-101, "failed to pthread_mutex_init, %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
mutex_destroy (mutex_t *mutex)
{
	if (unlikely(mutex == NULL))
		LOG_ERROR_AND_RETURN(-1, "null mutex_t");
	
	int error;
	
	mutex->lock = 0;
	mutex->type = 0;
	
	if (unlikely(0 != (error = pthread_mutex_destroy(&mutex->pmutex))))
		LOG_ERROR_AND_RETURN(-101, "failed to pthread_mutex_destroy, %d", error);
	
	return 0;
}

/**
 * No recursive locking support; will result in a dead-lock.
 *
 */
int
mutex_lock (mutex_t *mutex)
{
	if (unlikely(mutex == NULL))
		LOG_ERROR_AND_RETURN(-1, "null mutex_t");
	
	/*
	int error;
	
	int32_t volatile *lock = &mutex->lock;
	int32_t okay = 0;
	
	do {
		okay = ATOMIC_CAS32_BARRIER(0, 1, lock);
	}
	while (!okay);
	*/
	
	assert(!pthread_mutex_lock(&mutex->pmutex));
	
	/*
	if (unlikely(0 != (error = pthread_mutex_lock(&mutex->pmutex))))
		LOG_ERROR_AND_RETURN(-101, "failed to pthread_mutex_lock, %d", error);
	*/
	
	return 0;
}

/**
 *
 *
 */
int
mutex_unlock (mutex_t *mutex)
{
	if (unlikely(mutex == NULL))
		LOG_ERROR_AND_RETURN(-1, "null mutex_t");
	
	/*
	mutex->lock = 0;
	*/
	
	assert(!pthread_mutex_unlock(&mutex->pmutex));
	
	/*
	if (unlikely(0 != (error = pthread_mutex_unlock(&mutex->pmutex))))
		LOG_ERROR_AND_RETURN(-101, "failed to pthread_mutex_unlock, %d", error);
	*/
	
	return 0;
}

/**
 *
 *
 */
int32_t
mutex_lock2 (mutex_t *mutex)
{
	if (unlikely(mutex == NULL))
		LOG_ERROR_AND_RETURN(-1, "null mutex_t");
	
	int32_t lock = 0;
	
	lock = (ATOMIC_INC32_BARRIER(&mutex->last) - 1) % 10;
	
	while (0 == mutex->locks[lock])
		;
	
	mutex->locks[lock] = 0;
	
	return lock;
}

/**
 *
 *
 */
int
mutex_unlock2 (mutex_t *mutex, int32_t lock)
{
	if (unlikely(mutex == NULL))
		LOG_ERROR_AND_RETURN(-1, "null mutex_t");
	
	mutex->locks[(lock+1)%10] = 1;
	
	return 0;
}
