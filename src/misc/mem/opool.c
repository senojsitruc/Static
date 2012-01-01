/*
 *  opool.c
 *  Chatter
 *
 *  Created by Curtis Jones on 2009.03.26.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "opool.h"
#include "../atomic.h"
#include "../logger.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

static inline int opool_addblock (opool_t*);





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
opool_init (opool_t *pool, uint64_t osize, uint64_t bsize, uint64_t bmax, char *name)
{
	int error;
	size_t name_l;
	
	if (unlikely(pool == NULL))
		LOG_ERROR_AND_RETURN(-1, "null opool_t");
	
	if (unlikely(osize == 0))
		LOG_ERROR_AND_RETURN(-2, "invalid object size (0)");
	
	if (unlikely(bsize == 0))
		LOG_ERROR_AND_RETURN(-3, "invalid block size (0)");
	
	if (unlikely(bmax == 0))
		LOG_ERROR_AND_RETURN(-4, "invalid max blocks (0)");
	
	if (unlikely(name == NULL || (name_l = strlen(name)) == 0))
		LOG_ERROR_AND_RETURN(-5, "null name");
	
	if (unlikely(0 != (error = cobject_init(&pool->cobject, name, (cobject_destroy_func)opool_destroy, NULL))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, 0x%08X [%d]", error, error);
	
	if (unlikely(NULL == (pool->list = (void**)malloc(sizeof(void*) * bsize * bmax))))
		LOG_ERROR_AND_RETURN(-102, "[%s] failed to malloc(%lld), %s", name, (sizeof(void*)*bsize*bmax), strerror(errno));
	
	if (name_l >= sizeof(pool->name))
		name_l = sizeof(pool->name) - 1;
	
	memcpy(pool->name, name, name_l);
	
	pool->osize = osize;
	pool->bsize = bsize;
	pool->bmax = bmax;
	pool->bcount = 0;
	pool->ocount = 0;
	pool->size = (int64_t)(bsize * bmax);
	pool->head = 0;
	pool->tail = 0;
	pool->count = 0;
	
	memset(pool->list, 0, sizeof(void*) * bsize * bmax);
	
	LOG3("[%s] osize=%llu, bsize=%llu, bmax=%llu, total=%llu", pool->name, osize, bsize, bmax, (osize*bsize*bmax));
	
	if (unlikely(0 != (error = opool_addblock(pool))))
		LOG_ERROR_AND_RETURN(-102, "failed to opool_addblock, 0x%08X [%d]", error, error);
	
	return 0;
}

/**
 *
 *
 */
int
opool_init2 (opool_t *pool, uint64_t osize, uint64_t bsize, uint64_t bmax, char *name, opool_object_init_func init)
{
	int error;
	
	if (unlikely(pool == NULL))
		LOG_ERROR_AND_RETURN(-1, "null opool_t");
	
	if (unlikely(0 != (error = opool_init(pool, osize, bsize, bmax, name))))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_init, 0x%08X [%d]", error, error);
	
	pool->init = init;
	
	return 0;
}

/**
 *
 *
 */
int
opool_destroy (opool_t *pool)
{
	int error;
	
	if (unlikely(pool == NULL))
		LOG_ERROR_AND_RETURN(-1, "null opool_t");
	
	// TODO: deallocate all of the blocks
	
	if (unlikely(0 != (error = cobject_destroy(&pool->cobject))))
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
opool_push (opool_t *pool, void *object)
{
	int64_t count, head, size;
	
	if (unlikely(pool == NULL))
		LOG_ERROR_AND_RETURN(-1, "null opool_t");
	
	if (unlikely(object == NULL))
		LOG_ERROR_AND_RETURN(-2, "null object");
	
	if (pool->count >= pool->size)
		LOG_ERROR_AND_RETURN(-101, "[%s] pool is full (%llu)", pool->name, pool->count);
	
	// we're probably going to need the size later, so start fetching it now
	size = pool->size;
	
	// this is a very long way of saying, "pool->count++" while bounds checking against the pool size
	while (1) {
		count = pool->count;
		
		if (count >= pool->size)
			LOG_ERROR_AND_RETURN(-102, "[%s] pool is full (%llu)", pool->name, count);
		
		if (ATOMIC_CAS64_BARRIER(count, count+1, &pool->count))
			break;
	}
	
	// this is a very long way of saying, "pool->head++" and getting the original value.
	while (1) {
		head = pool->head;
		
		if (ATOMIC_CAS64_BARRIER(head, head+1, &pool->head))
			break;
	}
	
	// store the object in the pointer list
	pool->list[head % size] = object;
	
	return 0;
}

/**
 *
 *
 */
inline int
opool_pop (opool_t *pool, void **object)
{
	int error = 0;
	int64_t tail, size;
	
	if (unlikely(pool == NULL))
		LOG_ERROR_AND_RETURN(-1, "null opool");
	
	if (unlikely(object == NULL))
		LOG_ERROR_AND_RETURN(-2, "[%s] null object", pool->name);
	
	// clear out the void** return value, just in case
	*object = NULL;
	
	// if the list is empty, give up now before entering the loop
	if (pool->tail >= pool->head)
		return 0;
	
	// we're probably going to need the size later, so start fetching it now
	size = pool->size;
	
	// this is a very long way of saying, "pool->tail++". and we keep the original tail value, not
	// the incremented tail value.
	while (1) {
		tail = pool->tail;
		
		if (tail >= pool->head || pool->list[tail % size] == NULL) {
			if (unlikely(0 != (error = opool_addblock(pool))))
				goto done;
			continue;
		}
		
		if (ATOMIC_CAS64_BARRIER(tail, tail+1, &pool->tail))
			break;
	}
	
	*object = pool->list[tail % size];
	pool->list[tail % size] = NULL;
	memset(*object, 0, pool->osize);
	
	// pool->count--
	ATOMIC_DEC64_BARRIER(&pool->count);
	
done:
	return error;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline opool_t*
opool_retain (opool_t *opool)
{
	if (unlikely(opool == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null opool_t");
	
	return (opool_t*)cobject_retain(&opool->cobject);
}

/**
 *
 *
 */
inline void
opool_release (opool_t *opool)
{
	if (unlikely(opool == NULL))
		LOG_ERROR_AND_RETURN(, "null opool_t");
	
	cobject_release(&opool->cobject);
}





#pragma mark -
#pragma mark static functions

/**
 *
 *
 */
inline static int
opool_addblock (opool_t *pool)
{
	void *block;
	int64_t bcount;
	uint64_t bytes;
	
	if (unlikely(pool == NULL))
		LOG_ERROR_AND_RETURN(-1, "null opool_t");
	
	// make sure that we have not already met (or exceeded) the maximum
	// number of blocks. increment the number of blocks.
	while (1) {
		bcount = pool->bcount;
		
		if (bcount >= (int64_t)pool->bmax)
			LOG_ERROR_AND_RETURN(-101, "[%s] max blocks allocated (%llu)", pool->name, pool->bcount);
		
		if (ATOMIC_CAS64_BARRIER(bcount, bcount+1, (int64_t*)&pool->bcount))
			break;
	}
	
	// the number of bytes we need to allocate is equal to the size of a single object (in bytes)
	// times the number of objects in a block.
	bytes = pool->osize * pool->bsize;
	
	// allocate the block
	if (unlikely(NULL == (block = malloc(bytes))))
		LOG_ERROR_AND_RETURN(-102, "[%s] failed to malloc, %s", pool->name, strerror(errno));
	
	// zero the memory
	memset(block, 0, bytes);
	
	// for each object in the block, add it to the pool
	for (uint64_t i = 0; i < pool->bsize; ++i) {
		opool_push(pool, block);
		block += pool->osize;
		ATOMIC_INC64_BARRIER(&pool->ocount);
	}
	
	// TODO: keeep a list of blocks
	
	return 0;
}
