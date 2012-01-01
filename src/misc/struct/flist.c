/*
 *  flist.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#include "flist.h"
#include "../atomic.h"
#include "../logger.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
flist_init (flist_t *flist, uint32_t size, opool_t *pool)
{
	int error;
	
	if (unlikely(flist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null flist_t");
	
	if (unlikely(size == 0 || size > 10000))
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)flist, "flist", (cobject_destroy_func)flist_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, %d", error);
	
	if (unlikely(NULL == (flist->list = (cobject_t**)malloc(sizeof(cobject_t*) * size))))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc(%lu), %s", (sizeof(cobject_t*)*size), strerror(errno));
	
	flist->size = size;
	flist->head = 0;
	flist->tail = 0;
	flist->count = 0;
	
	memset(flist->list, 0, sizeof(cobject_t*)*size);
	
	return 0;
}

/**
 *
 *
 */
int
flist_destroy (flist_t *flist)
{
	int error;
	void *list;
	
	if (unlikely(flist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null flist_t");
	
	if (unlikely(0 != (error = flist_clear(flist))))
		LOG_ERROR_AND_RETURN(-101, "failed to flist_clear, %d", error);
	
	list = flist->list;
	
	if (list != NULL && ATOMIC_CASPTR_BARRIER(list, NULL, (void**)&flist->list))
		free(list);
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)flist))))
		LOG_ERROR_AND_RETURN(-102, "failed to cobject_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
inline int
flist_push (flist_t *flist, cobject_t *cobject)
{
	int64_t count, head, size;
	
	if (unlikely(flist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null flist_t");
	
	if (unlikely(cobject == NULL))
		LOG_ERROR_AND_RETURN(-2, "null cobject_t");
	
	if (flist->count >= flist->size)
		LOG_ERROR_AND_RETURN(-101, "list is full (%llu)", flist->count);
	
	// we're probably going to need the size later, so start fetching it now
	size = flist->size;
	
	// this is a very long way of saying, "flist->count++" while bounds checking against the flist
	// size
	while (1) {
		count = flist->count;
		
		if (count >= flist->size)
			LOG_ERROR_AND_RETURN(-102, "list is full (%llu)", count);
		
		if (ATOMIC_CAS64_BARRIER(count, count+1, &flist->count))
			break;
	}
	
	// this is a very long way of saying, "flist->head++" and getting the original value.
	while (1) {
		head = flist->head;
		
		if (ATOMIC_CAS64_BARRIER(head, head+1, &flist->head))
			break;
	}
	
	flist->list[head % size] = cobject_retain(cobject);
	
	return 0;
}

/**
 *
 *
 */
inline int
flist_pop (flist_t *flist, cobject_t **cobject)
{
	int error = 0;
	int64_t tail, size;
	
	if (unlikely(flist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null flist_t");
	
	if (unlikely(cobject == NULL))
		LOG_ERROR_AND_RETURN(-2, "null cobject_t");
	
	// clear out the cobject_t** return value, just in case
	*cobject = NULL;
	
	// if the list is empty, give up now before entering the loop
	if (flist->tail >= flist->head)
		return 0;
	
	// we're probably going to need the size later, so start fetching it now
	size = flist->size;
	
	// this is a very long way of saying, "flist->tail++". and we keep the original tail value, not
	// the incremented tail value.
	while (1) {
		tail = flist->tail;
		
		if (tail >= flist->head || flist->list[tail % size] == NULL)
			goto done;
		
		if (ATOMIC_CAS64_BARRIER(tail, tail+1, &flist->tail))
			break;
	}
	
	*cobject = flist->list[tail % size];
	flist->list[tail % size] = NULL;
	
	// flist->count--
	ATOMIC_DEC64_BARRIER(&flist->count);
	
done:
	return error;
}

/**
 *
 *
 */
inline int
flist_clear (flist_t *flist)
{
	cobject_t *cobject;
	
	if (unlikely(flist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null flist_t");
	
	while (0 == flist_pop(flist,&cobject) && cobject != NULL)
		cobject_release(cobject);
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline flist_t*
flist_retain (flist_t *flist)
{
	if (unlikely(flist == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null flist_t");
	
	return (flist_t*)cobject_retain((cobject_t*)flist);
}

/**
 *
 *
 */
inline void
flist_release (flist_t *flist)
{
	if (unlikely(flist == NULL))
		LOG_ERROR_AND_RETURN(, "null flist_t");
	
	cobject_release((cobject_t*)flist);
}
