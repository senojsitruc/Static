/*
 *  semaphore.h
 *  Static
 *
 *  Created by Curtis Jones on 2008.10.20.
 *  Copyright 2008 Curtis Jones. All rights reserved.
 *
 */

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "/usr/include/semaphore.h"
#include "../mem/cobject.h"
#include "../mem/memlock.h"

struct opool;

//
// semphore
//
struct semaphore
{
	cobject_t cobject;								// super class
	
	sem_t *sem;												// semaphore
	char name[20];										// semaphore name
};
typedef struct semaphore semaphore1_t;





/**
 *
 */
int semaphore_init (semaphore1_t*, struct opool*);

/**
 *
 */
int semaphore_destroy (semaphore1_t*);





/**
 *
 */
int semaphore_post (semaphore1_t*);

/**
 *
 */
int semaphore_wait (semaphore1_t*);





/**
 *
 */
semaphore1_t* semaphore_retain (semaphore1_t*);

/**
 *
 */
void semaphore_release (semaphore1_t*);

#endif /* __SEMAPHORE_H__ */
