/*
 *  datastream.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __DATA_STREAM_H__
#define __DATA_STREAM_H__

#include "dataobject.h"
#include "../misc/struct/flist.h"
#include "../misc/thread/thread.h"
#include <stdint.h>
#include <unistd.h>

//
// datastream_type
//
typedef enum
{
	DATASTREAM_RD = (1<<0),
	DATASTREAM_WR = (1<<1),
	DATASTREAM_RW = (1<<2)
} datastream_type;

//
// datastream
//
struct datastream
{
	datastream_type type;							// read, write read-write
	uint64_t key;											// implementation specific
	void *context;										// implementation specific
	
	uint32_t stop;										// stop the thread
	flist_t queue;										// incoming data queue
	thread1_t thread;									// flist_pop() thread
	uint32_t size;										// data size
	void *data;												// data copy
	
	/* callbacks */
	int (*feed)(void*, uint32_t, void*);
};
typedef struct datastream datastream_t;

//
// datastream_feed_func
//
//   context, size, data
//
typedef int (*datastream_feed_func)(void*, uint32_t, void*);





/**
 * datastream, context, size, function pointer
 */
int datastream_init (datastream_t*, void*, uint32_t, datastream_feed_func);

/**
 *
 */
int datastream_destroy (datastream_t*);





/**
 *
 */
int datastream_stop (datastream_t*);

/**
 * Called by the data source (ie, a device_t) to deliver a chunk of data.
 */
int datastream_feed (datastream_t*, dataobject_t*);

#endif /* __DATA_STREAM_H__ */
