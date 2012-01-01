/*
 *  eventnumber.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.17.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  A simple event used to send a numerical value.
 *
 */

#ifndef __EVENT_NUMBER_H__
#define __EVENT_NUMBER_H__

#include "event.h"
#include "../../misc/mem/cobject.h"
#include "../../misc/mem/opool.h"

//
// eventnumber
//
struct eventnumber
{
	event_t event;										// parent class
	
	union {
		uint8_t uint8val;								// unsigned 8-bit integer
		int8_t int8val;									// signed 8-bit integer
		uint16_t uint16val;							// unsigned 16-bit integer
		int16_t int16val;								// signed 16-bit integer
		uint32_t uint32val;							// unsigned 32-bit integer
		int32_t int32val;								// signed 32-bit integer
		uint64_t uint64val;							// unsigned 64-bit integer
		int64_t int64val;								// signed 64-bit integer
		float floatval;									// float
		double doubleval;								// double
	} val;
};
typedef struct eventnumber eventnumber_t;





/**
 * eventnumber, sender, type, pool
 */
int eventnumber_init (eventnumber_t*, void*, uint64_t, opool_t*);

/**
 *
 */
int eventnumber_destroy (eventnumber_t*);





/**
 *
 */
eventnumber_t* eventnumber_retain (eventnumber_t*);

/**
 *
 */
void eventnumber_release (eventnumber_t*);

#endif /* __EVENT_NUMBER_H__ */
