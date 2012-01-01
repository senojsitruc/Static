/*
 *  slist.c
 *  Chatter
 *
 *  Created by Curtis Jones on 2009.10.09.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "slist.h"
#include "../logger.h"

/**
 *
 *
 */
int
slist_init (slist_t *slist, opool_t *pool)
{
	if (unlikely(slist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null slist_t");
	
	if (unlikely(pool == NULL))
		LOG_ERROR_AND_RETURN(-2, "null opool_t");
	
	slist->head = NULL;
	slist->tail = NULL;
	slist->count = 0;
	slist->pool = opool_retain(pool);
	
	return 0;
}

/**
 *
 *
 */
int
slist_destroy (slist_t *slist)
{
	int error;
	
	if (unlikely(slist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null slist_t");
	
	if (unlikely(0 != (error = slist_clear(slist))))
		LOG_ERROR_AND_RETURN(-101, "failed to slist_clear, %d", error);
	
	opool_release(slist->pool);
	slist->pool = NULL;
	
	return 0;
}

/**
 *
 *
 */
int
slist_clear (slist_t *slist)
{
	cobject_t *object = NULL;
	
	if (unlikely(slist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null slist_t");
	
	while (0 == slist_pop(slist, &object) && object != NULL) {
		cobject_release(object);
	}
	
	return 0;
}

/**
 *
 *
 */
inline int
slist_push (slist_t *slist, cobject_t *object)
{
	int error;
	slist_item_t *item;
	
	if (unlikely(slist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null slist_t");
	
	if (unlikely(object == NULL))
		LOG_ERROR_AND_RETURN(-2, "null data");
	
	if (unlikely(0 != (error = opool_pop(slist->pool, (void**)&item)) || item == NULL))
		LOG_ERROR_AND_RETURN(-101, "failed to opool_pop, 0x%08X [%d]", error, error);
	
	item->data = cobject_retain(object);
	item->next = NULL;
	
	if (slist->tail != NULL)
		slist->tail->next = item;
	
	if (slist->head == NULL)
		slist->head = item;
	
	slist->tail = item;
	slist->count++;
	
	return 0;
}

/**
 * Don't forget: it is up to the caller of slist_pop() to release the memory.
 *
 */
inline int
slist_pop (slist_t *slist, cobject_t **object)
{
	slist_item_t *item;
	
	if (unlikely(slist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null slist_t");
	
	if (unlikely(object == NULL))
		LOG_ERROR_AND_RETURN(-2, "null cobject_t");
	
	if (slist->head != NULL) {
		item = slist->head;
		*object = item->data;
		slist->head = item->next;
		slist->count--;
		
		if (item == slist->tail)
			slist->tail = NULL;
		
		opool_push(slist->pool, item);
	}
	else
		*object = NULL;
	
	return 0;
}

/**
 *
 *
 */
inline int
slist_head (slist_t *slist, cobject_t **object)
{
	if (unlikely(slist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null slist_t");
	
	if (unlikely(object == NULL))
		LOG_ERROR_AND_RETURN(-2, "null cobject_t");
	
	if (slist->head != NULL)
		*object = slist->head->data;
	else
		*object = NULL;
	
	return 0;
}

/**
 *
 *
 */
inline int
slist_tail (slist_t *slist, cobject_t **object)
{
	if (unlikely(slist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null slist_t");
	
	if (unlikely(object == NULL))
		LOG_ERROR_AND_RETURN(-2, "null cobject_t");
	
	if (slist->tail != NULL)
		*object = slist->tail->data;
	else
		*object = NULL;
	
	return 0;
}

/**
 *
 *
 */
int
slist_peek (slist_t *slist, cobject_t **object, uint32_t index)
{
	slist_item_t *item;
	
	if (unlikely(slist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null slist_t");
	
	if (unlikely(object == NULL))
		LOG_ERROR_AND_RETURN(-2, "null cobject_t");
	
	*object = NULL;
	item = slist->head;
	
	// if the request index exceeds the list size, just return
	if (index >= slist->count || item == NULL)
		return 0;
	
	while (item != NULL && index > 0) {
		item = item->next;
		index--;
	}
	
	if (item != NULL)
		*object = item->data;
	
	return 0;
}





#pragma mark -
#pragma mark iterator stuff

/**
 *
 *
 */
inline int
slist_iter_init (slist_t *slist, slist_iter_t *iter)
{
	if (unlikely(slist == NULL))
		LOG_ERROR_AND_RETURN(-1, "null slist_t");
	
	if (unlikely(iter == NULL))
		LOG_ERROR_AND_RETURN(-2, "null slist_iter_t");
	
	iter->item = slist->head;
	
	return 0;
}

/**
 *
 *
 */
inline int
slist_iter_next (slist_iter_t *iter, cobject_t **object)
{
	if (unlikely(iter == NULL))
		LOG_ERROR_AND_RETURN(-1, "null slist_iter_t");
	
	if (unlikely(object == NULL))
		LOG_ERROR_AND_RETURN(-2, "null cobject_t");
	
	if (iter->item == NULL)
		*object = NULL;
	else {
		*object = iter->item->data;
		iter->item = iter->item->next;
	}
	
	return 0;
}
