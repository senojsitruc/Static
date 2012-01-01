/*
 *  rwfqueue.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.22.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "rwfqueue.h"
#include "../atomic.h"
#include "../logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

/**
 *
 *
 */
int
rwfqueue_init (rwfqueue_t *rwfqueue, uint32_t size, rwfqueue_mode rmode, rwfqueue_mode wmode)
{
	memset(rwfqueue, 0, sizeof(struct rwfqueue));
	
	if (unlikely(NULL == (rwfqueue->data = (rwfqueue_item_t *)malloc(sizeof(struct rwfqueue_item) * size))))
		LOG_ERROR_AND_RETURN(-1, "failed to malloc, %s", strerror(errno));
	
	rwfqueue->size = (int32_t)size;
	rwfqueue->count = 0;
	rwfqueue->rmode = rmode;
	rwfqueue->wmode = wmode;
	rwfqueue->head = rwfqueue->data;
	rwfqueue->tail = rwfqueue->data;
	rwfqueue->last = rwfqueue->data + (size-1);
	rwfqueue->stop = 0;
	
	memset(rwfqueue->data, 0, sizeof(struct rwfqueue_item) * size);
	
	if (0 != pthread_mutex_init(&rwfqueue->lock, NULL)) {
		LOG1("failed to pthread_mutex_init");
		rwfqueue_destroy(rwfqueue);
		return -2;
	}
	
	if (0 != pthread_cond_init(&rwfqueue->cond, NULL)) {
		LOG1("failed to pthread_cond_init");
		rwfqueue_destroy(rwfqueue);
		return -3;
	}
	
	return 0;
}

/**
 *
 *
 */
void
rwfqueue_destroy (rwfqueue_t *rwfqueue)
{
	if (rwfqueue == NULL)
		return;
	
	if (rwfqueue->data != NULL) {
		free(rwfqueue->data);
		rwfqueue->data = NULL;
	}
	
	pthread_mutex_destroy(&rwfqueue->lock);
	pthread_cond_destroy(&rwfqueue->cond);
	
	memset(rwfqueue, 0, sizeof(struct rwfqueue));
	
//free(rwfqueue);
}

/**
 *
 *
 */
uint32_t
rwfqueue_count (rwfqueue_t *rwfqueue)
{
	if (rwfqueue == NULL)
		return 0;
	
	return (uint32_t)rwfqueue->count;
}

/**
 *
 *
 */
int
rwfqueue_push (rwfqueue_t *rwfqueue, void *data)
{
	if (rwfqueue == NULL)
		return -1;
	
	if (data == NULL)
		return -2;
	
	if (rwfqueue->size == rwfqueue->count) {
		if (rwfqueue->wmode == RWFQUEUE_MODE_NOBLOCK)
			return -3;
		else {
			pthread_mutex_lock(&rwfqueue->lock);
			
			while (!rwfqueue->stop && rwfqueue->size == rwfqueue->count)
				pthread_cond_wait(&rwfqueue->cond, &rwfqueue->lock);
			
			pthread_mutex_unlock(&rwfqueue->lock);
		}
	}
	
	rwfqueue->head->data = data;
	rwfqueue->head->inuse = 1;
	
	ATOMIC_INC32(&rwfqueue->count);
	
	if (rwfqueue->head == rwfqueue->last)
		rwfqueue->head = rwfqueue->data;
	else
		rwfqueue->head++;
	
	// send a signal to the reader if there wasn't anything in
	// the queue prior to this call to push(), and now there is.
	if (rwfqueue->count == 1)
		pthread_cond_signal(&rwfqueue->cond);
	
	return 0;
}

/**
 *
 *
 */
void *
rwfqueue_pop (rwfqueue_t *rwfqueue)
{
	void *data = NULL;
	
	if (rwfqueue == NULL)
		return NULL;
	
	if (0 == rwfqueue->count) {
		if (rwfqueue->rmode == RWFQUEUE_MODE_NOBLOCK)
			return NULL;
		else {
			pthread_mutex_lock(&rwfqueue->lock);
			
			while (!rwfqueue->stop && 0 == rwfqueue->count)
				pthread_cond_wait(&rwfqueue->cond, &rwfqueue->lock);
			
			pthread_mutex_unlock(&rwfqueue->lock);
		}
	}
	
	data = rwfqueue->tail->data;
	
	rwfqueue->tail->data = NULL;
	rwfqueue->tail->inuse = 0;
	
	ATOMIC_DEC32(&rwfqueue->count);
	
	if (rwfqueue->tail == rwfqueue->last)
		rwfqueue->tail = rwfqueue->data;
	else
		rwfqueue->tail++;
	
	return data;
}

/**
 *
 *
 */
void *
rwfqueue_peek (rwfqueue_t *rwfqueue)
{
	if (rwfqueue == NULL)
		return NULL;
	
	if (0 == rwfqueue->count) {
		if (rwfqueue->rmode == RWFQUEUE_MODE_NOBLOCK)
			return NULL;
		else {
			pthread_mutex_lock(&rwfqueue->lock);
			
			while (!rwfqueue->stop && 0 == rwfqueue->count)
				pthread_cond_wait(&rwfqueue->cond, &rwfqueue->lock);
			
			pthread_mutex_unlock(&rwfqueue->lock);
		}
	}
	
	return rwfqueue->tail->data;
}

/**
 *
 *
 */
void *
rwfqueue_peak2 (rwfqueue_t *rwfqueue, int32_t index)
{
	if (rwfqueue == NULL)
		return NULL;
	
	if (index < 0)
		return NULL;
	
	if (index >= rwfqueue->size)
		return NULL;
	
	
	// TODO: tail wraps around, so we need to mod index, or something.
	
	
	return rwfqueue->tail[index].data;
//return ((rwfqueue_t*)(rwfqueue->tail + index))->data;
//return ((rwfqueue_t*)(rwfqueue->tail + (sizeof(rwfqueue_item_t) * index)))->data;
}

/**
 *
 *
 */
void *
rwfqueue_last (rwfqueue_t *rwfqueue)
{
	if (rwfqueue == NULL)
		return NULL;
	
	if (rwfqueue->tail == NULL)
		return NULL;
	
	return rwfqueue->tail->data;
}

/**
 *
 *
 */
void *
rwfqueue_head (rwfqueue_t *rwfqueue)
{
	if (rwfqueue == NULL)
		return NULL;
	
	if (rwfqueue->count == 0)
		return NULL;
	
	if (rwfqueue->head == rwfqueue->data)
		return rwfqueue->last->data;
	else
		return (rwfqueue->head - 1)->data;
}
