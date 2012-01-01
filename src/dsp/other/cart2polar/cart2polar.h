/*
 *  cart2polar.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  http://en.wikipedia.org/wiki/Polar_coordinate_system
 *
 */

#ifndef __CART2POLAR_H__
#define __CART2POLAR_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// cart2polar
//
struct cart2polar
{
	dsp_t dsp;												// parent class
};
typedef struct cart2polar cart2polar_t;





/**
 *
 */
int cart2polar_init (cart2polar_t*, opool_t*);

/**
 *
 */
int cart2polar_destroy (cart2polar_t*);





/**
 *
 */
int cart2polar_feed (cart2polar_t*, uint32_t*, double*);

/**
 *
 */
int cart2polar_reset (cart2polar_t*);





/**
 *
 */
cart2polar_t* cart2polar_retain (cart2polar_t*);

/**
 *
 */
void cart2polar_release (cart2polar_t*);

#endif /* __CART2POLAR_H__ */
