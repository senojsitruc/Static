/*
 *  semaphore.c
 *  Static
 *
 *  Created by Curtis Jones on 2008.10.20.
 *  Copyright 2008 Curtis Jones. All rights reserved.
 *
 */

#include "semaphore.h"
#include "../logger.h"
#include "../mem/opool.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static uint32_t gSemName;
static uint32_t gSemRand;

#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
semaphore_init (semaphore1_t *semaphore, opool_t *pool)
{
	int error;
	
	if (unlikely(semaphore == NULL))
		LOG_ERROR_AND_RETURN(-1, "null semaphore1_t");
	
	if (gSemRand == 0)
		gSemRand = (uint32_t)random() % 10000;
	
	snprintf(semaphore->name, sizeof(semaphore->name), "static-%04x-%04d", gSemRand, ++gSemName);
	
	if (unlikely(0 != (error = cobject_init(&semaphore->cobject, semaphore->name, (cobject_destroy_func)semaphore_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, 0x%08X [%d]", error, error);
	
	if (SEM_FAILED == (semaphore->sem = sem_open(semaphore->name, O_CREAT | O_EXCL, S_IRWXU, 0)))
		LOG_ERROR_AND_RETURN(-102, "failed to sem_open(%s), %s", semaphore->name, strerror(errno));
	
	// if we unlink it now, we don't have to worry about not unlinking it later
	if (-1 == sem_unlink(semaphore->name))
		LOG_ERROR_AND_RETURN(-103, "failed to sem_unlink(%s), %s", semaphore->name, strerror(errno));
	
	//LOG3("opened semaphore, %s", semaphore->name);
	
	return 0;
}

/**
 *
 *
 */
int
semaphore_destroy (semaphore1_t *semaphore)
{
	int error;
	
	if (unlikely(semaphore == NULL))
		LOG_ERROR_AND_RETURN(-1, "null semaphore1_t");
	
	if (unlikely(semaphore->sem != NULL && 0 != (error = sem_close(semaphore->sem))))
		LOG_ERROR_AND_RETURN(-101, "failed to sem_close, %s", strerror(errno));
	
	if (-1 == sem_unlink(semaphore->name))
		LOG_ERROR_AND_RETURN(-102, "failed to sem_unlink(%s), %s", semaphore->name, strerror(errno));
	
	if (unlikely(0 != (error = cobject_destroy(&semaphore->cobject))))
		LOG_ERROR_AND_RETURN(-103, "failed to cobject_destroy, 0x%08X [%d]", error, error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
inline int
semaphore_post (semaphore1_t *semaphore)
{
	int error;
	
	if (unlikely(semaphore == NULL))
		LOG_ERROR_AND_RETURN(-1, "null semaphore1_t");
	
	if (unlikely(0 != (error = sem_post(semaphore->sem))))
		LOG_ERROR_AND_RETURN(-101, "failed to sem_post, %s", strerror(errno));
	
	return 0;
}

/**
 *
 *
 */
inline int
semaphore_wait (semaphore1_t *semaphore)
{
	int error;
	
	if (unlikely(semaphore == NULL))
		LOG_ERROR_AND_RETURN(-1, "null semaphore1_t");
	
	if (unlikely(0 != (error = sem_wait(semaphore->sem))))
		LOG_ERROR_AND_RETURN(-101, "failed to sem_wait, %s", strerror(errno));
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline semaphore1_t*
semaphore_retain (semaphore1_t *semaphore)
{
	if (unlikely(semaphore == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null semaphore1_t");
	
	return (semaphore1_t*)cobject_retain(&semaphore->cobject);
}

/**
 *
 *
 */
inline void
semaphore_release (semaphore1_t *semaphore)
{
	if (unlikely(semaphore == NULL))
		LOG_ERROR_AND_RETURN(, "null semaphore1_t");
	
	cobject_release(&semaphore->cobject);
}
