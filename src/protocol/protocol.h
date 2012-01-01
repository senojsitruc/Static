/*
 *  protocol.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.18.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>

//
// protocol
//
struct protocol
{
	char name[50];										// protocol name
};
typedef struct protocol protocol_t;

/**
 *
 */
int protocol_init (protocol_t*, char*);

/**
 *
 */
int protocol_destroy (protocol_t*);

#endif /* __PROTOCOL_H__ */
