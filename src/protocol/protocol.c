/*
 *  protocol.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "protocol.h"
#include "../misc/logger.h"
#include <string.h>

/**
 *
 *
 */
int
protocol_init (protocol_t *protocol, char *name)
{
	size_t name_l;
	
	if (unlikely(protocol == NULL))
		LOG_ERROR_AND_RETURN(-1, "null protocol_t");
	
	if (unlikely(name == NULL || (name_l = strlen(name)) == 0))
		LOG_ERROR_AND_RETURN(-2, "null or zero-length name");
	
	if (unlikely(name_l >= sizeof(protocol->name)))
		LOG_ERROR_AND_RETURN(-3, "invalid name length, %lu (max=%lu)", name_l, (sizeof(protocol->name)-1));
	
	memcpy(protocol->name, name, name_l);
	
	return 0;
}

/**
 *
 *
 */
int
protocol_destroy (protocol_t *protocol)
{
	if (unlikely(protocol == NULL))
		LOG_ERROR_AND_RETURN(-1, "null protocol_t");
	
	return 0;
}
