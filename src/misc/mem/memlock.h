/*
 *  memlock.h
 *  Static
 *
 *  Created by Curtis Jones on 2008.10.20.
 *  Copyright 2008 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------
 *
 *  This class is a reference counting and garbage collection 
 *  mechanism, of sorts. If the 'destroy' parameter is specified in
 *  the call to init(), when the reference count reaches zero, that
 *  function is called and the 'object' parameter is passed to it.
 *  This is the means by which the class using a memlock can know 
 *  that it should free up any resources it has consumed.
 *
 */

#ifndef __MEM_LOCK_H__
#define __MEM_LOCK_H__

#include <stdint.h>

//
// memlock
//
struct memlock
{
	int32_t volatile inuse;						// current reference count
	int (*destroy)(void*);						// destroy function pointer
	void *object;											// context object
	struct memlock *next;							// memlock release list
};
typedef struct memlock memlock_t;

//
// memlock_destroy_func
//
typedef int (*memlock_destroy_func)(void*);

/**
 *
 */
int memlock_setup ();

/**
 *
 */
int memlock_init (memlock_t*, memlock_destroy_func, void*);

/**
 *
 */
int memlock_destroy (memlock_t*);

/**
 *
 */
void memlock_retain (memlock_t*);

/**
 *
 */
void memlock_release (memlock_t*);

#endif /* __MEM_LOCK_H__ */
