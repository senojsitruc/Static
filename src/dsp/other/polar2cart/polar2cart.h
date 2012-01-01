/*
 *  polar2cart.h
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

#ifndef __POLAR2CART_H__
#define __POLAR2CART_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// polar2cart
//
struct polar2cart
{
	dsp_t dsp;												// parent class
};
typedef struct polar2cart polar2cart_t;





/**
 *
 */
int polar2cart_init (polar2cart_t*, opool_t*);

/**
 *
 */
int polar2cart_destroy (polar2cart_t*);





/**
 *
 */
int polar2cart_feed (polar2cart_t*, uint32_t*, double*);

/**
 *
 */
int polar2cart_reset (polar2cart_t*);





/**
 *
 */
polar2cart_t* polar2cart_retain (polar2cart_t*);

/**
 *
 */
void polar2cart_release (polar2cart_t*);

#endif /* __POLAR2CART_H__ */
