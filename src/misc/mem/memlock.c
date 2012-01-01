/*
 *  memlock.c
 *  Static
 *
 *  Created by Curtis Jones on 2008.10.20.
 *  Copyright 2008 Curtis Jones. All rights reserved.
 *
 */

#include "memlock.h"
#include "../atomic.h"
#include "../logger.h"
#include "../thread/semaphore.h"
#include "../thread/thread.h"
#include <string.h>

static thread1_t gThread;
static semaphore1_t gSemaphore;
static int32_t volatile gCount;
static int32_t gStop;
static memlock_t *gHead;

void __memlock_push (memlock_t*);
void* __memlock_thread (void*);

#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
memlock_setup ()
{
	int error;
	
	memset(&gThread, 0, sizeof(thread1_t));
	memset(&gSemaphore, 0, sizeof(semaphore1_t));
	
	gCount = 0;
	gHead = NULL;
	gStop = 0;
	
	if (unlikely(0 != (error = semaphore_init(&gSemaphore, NULL))))
		LOG_ERROR_AND_RETURN(-101, "failed to semaphore_init, 0x%08X [%d]", error, error);
	
	if (unlikely(0 != (error = thread_init(&gThread, "memlock-thread", (thread_start_func)__memlock_thread, NULL, NULL))))
		LOG_ERROR_AND_RETURN(-102, "failed to thread_init, 0x%08X [%d]", error, error);
	
	if (unlikely(0 != (error = thread_start(&gThread))))
		LOG_ERROR_AND_RETURN(-103, "failed to therad_start, 0x%08X [%d]", error, error);
	
	return error;
}

/**
 *
 *
 */
inline int
memlock_init (memlock_t *memlock, memlock_destroy_func destroy, void *object)
{
	if (unlikely(memlock == NULL))
		LOG_ERROR_AND_RETURN(-1, "null memlock_t");
	
	memset(memlock, 0, sizeof(memlock_t));
	
	memlock->inuse = 1;
	memlock->destroy = destroy;
	memlock->object = object;
	
	return 0;
}

/**
 *
 *
 */
inline int
memlock_destroy (memlock_t *memlock)
{
	if (unlikely(memlock == NULL))
		LOG_ERROR_AND_RETURN(-1, "null memlock_t");
	
	memlock->inuse = 0;
	memlock->destroy = 0;
	memlock->object = 0;
	memlock->next = NULL;
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
inline void
memlock_retain (memlock_t *memlock)
{
	if (unlikely(memlock == NULL))
		LOG_ERROR_AND_RETURN(, "null memlock_t");
	
	ATOMIC_INC32_BARRIER(&memlock->inuse);
}

/**
 * When the final call to release() is made such that we perform the
 * destroy() callback, do not do anything else in here because that
 * callback probably also (and rightly) called memlock_destroy().
 *
 */
inline void
memlock_release (memlock_t *memlock)
{
	int inuse;
	
	if (unlikely(memlock == NULL))
		LOG_ERROR_AND_RETURN(, "null memlock_t");
	
	inuse = ATOMIC_DEC32_BARRIER(&memlock->inuse);
	
	if (inuse == 0)
		if (memlock->destroy != NULL)
			__memlock_push(memlock);
}





#pragma mark -
#pragma mark private

/**
 *
 *
 */
inline void
__memlock_push (memlock_t *memlock)
{
	memlock_t *next = NULL;
	
	if (unlikely(memlock == NULL))
		LOG_ERROR_AND_RETURN(, "null memlock_t");
	
	while (1) {
		next = memlock->next = gHead;
		
		if (ATOMIC_CASPTR_BARRIER(memlock->next, memlock, (void**)&gHead)) {
			if (next == NULL)
				semaphore_post(&gSemaphore);
			ATOMIC_INC32_BARRIER(&gCount);
			break;
		}
	}
}

/**
 *
 *
 */
void*
__memlock_thread (void *arg)
{
	memlock_t *memlock, *next;
	
	while (0 == gStop) {
		if (gHead == NULL) {
			semaphore_wait(&gSemaphore);
			continue;
		}
		
		while (1) {
			if (NULL == (memlock = gHead))
				break;
			
			next = memlock->next;
			
			if (ATOMIC_CASPTR_BARRIER(memlock, next, (void**)&gHead)) {
				ATOMIC_DEC32_BARRIER(&gCount);
				break;
			}
		}
		
		if (memlock->destroy != NULL)
			(*memlock->destroy)(memlock->object);
	}
	
	return NULL;
}
