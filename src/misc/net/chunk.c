/*
 *  chunk.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.22.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "chunk.h"
#include <string.h>
#include "../logger.h"

/**
 *
 *
 */
inline int
chunk_init (chunk_t *chunk, opool_t *pool)
{
	int error;
	
	if (unlikely(chunk == NULL))
		LOG_ERROR_AND_RETURN(-1, "null chunk_t");
	
	if (unlikely(0 != (error = cobject_init((cobject_t*)chunk, "chunk", (cobject_destroy_func)chunk_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_init, %d", error);
	
	return 0;
}

/**
 *
 *
 */
inline int
chunk_destroy (chunk_t *chunk)
{
	int error;
	
	if (unlikely(chunk == NULL))
		LOG_ERROR_AND_RETURN(-1, "null chunk_t");
	
	if (unlikely(0 != (error = cobject_destroy((cobject_t*)chunk))))
		LOG_ERROR_AND_RETURN(-101, "failed to cobject_destroy, %d", error);
	
	return 0;
}

/**
 *
 *
 */
inline int
chunk_reset (chunk_t *chunk)
{
	if (unlikely(chunk == NULL))
		LOG_ERROR_AND_RETURN(-1, "null chunk_t");
	
	chunk->leng = 0;
	chunk->offs = 0;
	memset((void*)chunk->data, 0, sizeof(chunk->data));
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline chunk_t*
chunk_retain (chunk_t *chunk)
{
	if (unlikely(chunk == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null chunk_t");
	
	return (chunk_t*)cobject_retain((cobject_t*)chunk);
}

/**
 *
 *
 */
inline void
chunk_release (chunk_t *chunk)
{
	if (unlikely(chunk == NULL))
		LOG_ERROR_AND_RETURN(, "null chunk_t");
	
	cobject_release((cobject_t*)chunk);
}
