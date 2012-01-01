/*
 *  rectangle.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  http://en.wikipedia.org/wiki/Window_function#Rectangular_window
 *
 */

#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// rectangle
//
struct rectangle
{
	dsp_t dsp;												// parent class
	
	uint32_t size;										// window size
	double offset;										// adjustment
	double *window;										// window
};
typedef struct rectangle rectangle_t;





/**
 * rectangle, window size, adjustment, pool
 */
int rectangle_init (rectangle_t*, uint32_t, double, opool_t*);

/**
 *
 */
int rectangle_destroy (rectangle_t*);





/**
 *
 */
int rectangle_feed (rectangle_t*, uint32_t*, double*);

/**
 *
 */
int rectangle_reset (rectangle_t*);





/**
 *
 */
rectangle_t* rectangle_retain (rectangle_t*);

/**
 *
 */
void rectangle_release (rectangle_t*);

#endif /* __WINDOW_RECTANGLE_H__ */
