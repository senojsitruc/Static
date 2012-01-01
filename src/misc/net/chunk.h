/*
 *  chunk.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.22.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __CHUNK_H__
#define __CHUNK_H__

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "../mem/cobject.h"
#include "../mem/memlock.h"
#include "../mem/opool.h"

//
// chunk
//
struct chunk
{
	cobject_t cobject;						// parent class
	
	uint32_t volatile leng;				// number of bytes of data in chunk
	uint32_t volatile offs;				// read offset (in bytes)
	uint8_t volatile data[16000];	// data segment
};
typedef struct chunk chunk_t;

/**
 * Sets up the given chunk_t to be used.
 */
int chunk_init (chunk_t*, opool_t*);

/**
 * Releases any locks, references, memory, etc.
 */
int chunk_destroy (chunk_t*);

/**
 * Put a chunk_t back in an unused state.
 */
int chunk_reset (chunk_t*);

/**
 * Retain the object.
 */
chunk_t* chunk_retain (chunk_t*);

/**
 * Release the object.
 */
void chunk_release (chunk_t*);

#endif /* __CHUNK_H__ */
