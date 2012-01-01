/*
 *  thread.c
 *  Static
 *
 *  Created by Curtis Jones on 2008.10.20.
 *  Copyright 2008 Curtis Jones. All rights reserved.
 *
 */

#include "thread.h"
#include "../logger.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static void* thread_run (thread1_t*);

#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
thread_init (thread1_t *thread, char *name, thread_start_func start_func, void *restrict arg, opool_t *pool)
{
	int error = 0;
	
	if (unlikely(thread == NULL))
		LOG_ERROR_AND_RETURN(-1, "null thread1_t");
	
	if (unlikely(0 != (error = cobject_init(&thread->cobject, name, (cobject_destroy_func)thread_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, 0x%08X [%d]", error, error);
	
	thread->start_func = start_func;
	thread->arg = arg;
	
	return 0;
}

/**
 *
 *
 */
int
thread_destroy (thread1_t *thread)
{
	int error;
	
	if (unlikely(thread == NULL))
		LOG_ERROR_AND_RETURN(-1, "null thread1_t");
	
	if (unlikely(0 != (error = cobject_destroy(&thread->cobject))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_destroy, 0x%08X [%d]", error, error);
	
	return 0;
}





#pragma mark -
#pragma mark thread stuff

/**
 *
 *
 */
int
thread_start (thread1_t *thread)
{
	int error;
	
	if (unlikely(thread == NULL))
		LOG_ERROR_AND_RETURN(-1, "null thread1_t");
	
	if (unlikely(thread->state & THREAD_STATE_STARTED))
		LOG_ERROR_AND_RETURN(-101, "thread is already running");
	
	if (unlikely(0 != (error = pthread_create(&thread->pthread, NULL, (thread_start_func)thread_run, thread))))
		LOG_ERROR_AND_RETURN(-102, "failed to pthread_create, 0x%08X [%d]", error, error);
	
	return 0;
}

/**
 *
 *
 */
int
thread_stop (thread1_t *thread)
{
	if (unlikely(thread == NULL))
		LOG_ERROR_AND_RETURN(-1, "null thread1_t");
	
	thread->state |= THREAD_STATE_STOPPING;
	thread_signal(thread, SIGALRM);
	
	return 0;
}

/**
 *
 *
 */
thread_state
thread_getstate (thread1_t *thread)
{
	if (unlikely(thread == NULL))
		LOG_ERROR_AND_RETURN(-1, "null thread1_t");
	
	return thread->state;
}

/**
 *
 *
 */
int
thread_signal (thread1_t *thread, int signum)
{
	if (unlikely(thread == NULL))
		LOG_ERROR_AND_RETURN(-1, "null thread1_t");
	
	return pthread_kill(thread->pthread, signum);
}

/**
 *
 *
 */
static void*
thread_run (thread1_t *thread)
{
	void *ret;
	
	thread->state = THREAD_STATE_STARTED;
	ret = (*thread->start_func)(thread->arg);
	thread->state = THREAD_STATE_STOPPED;
	
	return ret;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline thread1_t*
thread_retain (thread1_t *thread)
{
	if (unlikely(thread == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null thread1_t");
	
	return (thread1_t*)cobject_retain(&thread->cobject);
}

/**
 *
 *
 */
inline void
thread_release (thread1_t *thread)
{
	if (unlikely(thread == NULL))
		LOG_ERROR_AND_RETURN(, "null thread1_t");
	
	cobject_release(&thread->cobject);
}
