/*
 *  triangle.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  http://en.wikipedia.org/wiki/Window_function#Triangular_window_.28non-zero_end-points.29
 *
 */

#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// triangle
//
struct triangle
{
	dsp_t dsp;												// parent class
	
	uint32_t size;										// window size
	double offset;										// adjustment
	double *window;										// window
};
typedef struct triangle triangle_t;





/**
 * triangle, window size, adjustment, pool
 */
int triangle_init (triangle_t*, uint32_t, double, opool_t*);

/**
 *
 */
int triangle_destroy (triangle_t*);





/**
 *
 */
int triangle_feed (triangle_t*, uint32_t*, double*);

/**
 *
 */
int triangle_reset (triangle_t*);





/**
 *
 */
triangle_t* triangle_retain (triangle_t*);

/**
 *
 */
void triangle_release (triangle_t*);

#endif /* __WINDOW_TRIANGLE_H__ */
