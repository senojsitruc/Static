/*
 *  datastream.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "datastream.h"
#include "../misc/logger.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>





#pragma mark -
#pragma mark private methods

void* __datastream_thread (datastream_t*);





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
datastream_init (datastream_t *datastream, void *context, uint32_t size, datastream_feed_func feed_func)
{
	int error;
	
	if (unlikely(datastream == NULL))
		LOG_ERROR_AND_RETURN(-1, "null datasttream_t");
	
	if (unlikely(NULL == (datastream->data = malloc(size))))
		LOG_ERROR_AND_RETURN(-101, "failed to malloc(%u), %s", size, strerror(errno));
	
	if (unlikely(0 != (error = flist_init(&datastream->queue, 100, NULL))))
		LOG_ERROR_AND_RETURN(-102, "failed to flist_init, %d", error);
	
	if (unlikely(0 != (error = thread_init(&datastream->thread, "datastream-thread", (thread_start_func)__datastream_thread, datastream, NULL))))
		LOG_ERROR_AND_RETURN(-103, "failed to thread_init, %d", error);
	
	datastream->size = size;
	datastream->stop = 0;
	datastream->context = context;
	datastream->feed = feed_func;
	
	memset(datastream->data, 0, size);
	
	if (unlikely(0 != (error = thread_start(&datastream->thread))))
		LOG_ERROR_AND_RETURN(-103, "failed to thread_start, %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
datastream_destroy (datastream_t *datastream)
{
	int error;
	
	if (unlikely(datastream == NULL))
		LOG_ERROR_AND_RETURN(-1, "null datasttream_t");
	
	if (datastream->queue.count != 0)
		LOG3("queue is not empty! [count=%llu]", datastream->queue.count);
	
	if (unlikely(0 != (error = flist_destroy(&datastream->queue))))
		LOG_ERROR_AND_RETURN(-101, "failed to flist_destroy, %d", error);
	
	if (datastream->data != NULL) {
		free(datastream->data);
		datastream->data = NULL;
	}
	
	if (unlikely(0 != (error = thread_destroy(&datastream->thread))))
		LOG_ERROR_AND_RETURN(-101, "failed to thread_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark accessors

/**
 *
 *
 */
int
datastream_stop (datastream_t *datastream)
{
	if (unlikely(datastream == NULL))
		LOG_ERROR_AND_RETURN(-1, "null datastream_t");
	
	datastream->stop = 1;
	
	sleep(2);
	
	return 0;
}

/**
 * Pushes the data chunk onto the incoming data chunk queue. The datastream receive thread will see
 * the new data chunk, pop it off, make a copy, and pass it to the "feed" callback.
 */
int
datastream_feed (datastream_t *datastream, dataobject_t *dataobject)
{
	int error;
	
	if (unlikely(datastream == NULL))
		LOG_ERROR_AND_RETURN(-1, "null datastream_t");
	
	if (unlikely(dataobject == NULL))
		LOG_ERROR_AND_RETURN(-2, "null dataobject_t");
	
	// don't accept data for a stopped stream
	if (datastream->stop)
		LOG_ERROR_AND_RETURN(-101, "datastream is stopped. no more data, please");
	
	if (unlikely(0 != (error = flist_push(&datastream->queue, (cobject_t*)dataobject))))
		LOG_ERROR_AND_RETURN(-102, "failed to flist_push(), %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark private

/**
 * Waits for data objects to be pushed onto the incoming data queue. Pops a data object off the
 * queue and passes it to the 'feed' callback. Sleeps until more data is available. Repeat.
 *
 */
void*
__datastream_thread (datastream_t *datastream)
{
	int error;
	dataobject_t *dataobject;
	flist_t *queue;
	void *context;
	void *data;
	uint32_t size;
	
	if (unlikely(datastream == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null datastream_t");
	
	dataobject = NULL;
	data = datastream->data;
	context = datastream->context;
	queue = &datastream->queue;
	
	while (!datastream->stop) {
		if (unlikely(0 != (error = flist_pop(queue, (cobject_t**)&dataobject)))) {
			LOG_ERROR_AND_RETURN(NULL, " failed to flist_pop(), %d", error);
		}
		else if (dataobject == NULL) {
			usleep(10000);
			continue;
		}
		
		memcpy(data, dataobject->data, (size = MIN(datastream->size,dataobject->size)));
		
		dataobject_release(dataobject);
		dataobject = NULL;
		
		if (likely(datastream->feed != NULL))
			if (unlikely(0 != (error = (*datastream->feed)(context, size, data))))
				LOG3("failed to datastream->feed, %d", error);
	}
	
	// release any data objects in our queue
	while (0 == (error = flist_pop(queue, (cobject_t**)&dataobject)) && dataobject != NULL) {
		dataobject_release(dataobject);
		dataobject = NULL;
	}
	
	return datastream;
}
