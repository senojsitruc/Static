/*
 *  eventnumber.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.17.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "eventnumber.h"
#include "../logger.h"





#pragma mark -
#pragma mark structors

/**
 *
 *
 */
int
eventnumber_init (eventnumber_t *eventnumber, void *sender, uint64_t type, opool_t *pool)
{
	int error;
	
	if (unlikely(eventnumber == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventnumber_t");
	
	if (unlikely(0 != (error = event_init((event_t*)eventnumber, sender, type, "eventnumber", (cobject_destroy_func)eventnumber_destroy, pool))))
		LOG_ERROR_AND_RETURN(-101, "failed to event_init, %d", error);
	
	return 0;
}

/**
 *
 *
 */
int
eventnumber_destroy (eventnumber_t *eventnumber)
{
	int error;
	
	if (unlikely(eventnumber == NULL))
		LOG_ERROR_AND_RETURN(-1, "null eventnumber_t");
	
	if (unlikely(0 != (error = event_destroy((event_t*)eventnumber))))
		LOG_ERROR_AND_RETURN(-101, "failed to event_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
eventnumber_t*
eventnumber_retain (eventnumber_t *eventnumber)
{
	if (unlikely(eventnumber == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null eventnumber_t");
	
	return (eventnumber_t*)event_retain((event_t*)eventnumber);
}

/**
 *
 *
 */
void
eventnumber_release (eventnumber_t *eventnumber)
{
	if (unlikely(eventnumber == NULL))
		LOG_ERROR_AND_RETURN(, "null eventnumber_t");
	
	event_release((event_t*)eventnumber);
}
