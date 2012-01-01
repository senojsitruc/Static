/*
 *  dataobject.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 */

#ifndef __DATA_OBJECT_H__
#define __DATA_OBJECT_H__

#include "../misc/mem/cobject.h"
#include "../misc/mem/opool.h"
#include <stdint.h>

//
// dataobject
//
struct dataobject
{
	cobject_t cobject;                   // parent class
	
	uint32_t size;                       // size of data (in bytes)
	uint8_t data[10000];                 // data
};
typedef struct dataobject dataobject_t;





/**
 *
 */
int dataobject_init (dataobject_t*, opool_t*);

/**
 *
 */
int dataobject_destroy (dataobject_t*);





/**
 *
 */
dataobject_t* dataobject_retain (dataobject_t*);

/**
 *
 */
void dataobject_release (dataobject_t*);

#endif /* __DATA_OBJECT_H__ */
