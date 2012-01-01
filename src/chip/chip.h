/*
 *  chip.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.28.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __CHIP_H__
#define __CHIP_H__

#include "../misc/mem/cobject.h"
#include "../misc/mem/opool.h"

//
// chip_type
//
typedef enum
{
	CHIP_AD6620 = (1<<0)
} chip_type;

//
// chip
//
struct chip
{
	cobject_t cobject;								// parent class
	chip_type type;										// chip type
};
typedef struct chip chip_t;

/**
 * chip, chip type, name, destroy function pointer, pool
 */
int chip_init (chip_t*, chip_type, char*, cobject_destroy_func, opool_t*);

/**
 *
 */
int chip_destroy (chip_t*);





/**
 *
 */
chip_t* chip_retain (chip_t*);

/**
 *
 */
void chip_release (chip_t*);

#endif /* __CHIP_H__ */
