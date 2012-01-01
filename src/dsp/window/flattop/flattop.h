/*
 *  flattop.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  http://en.wikipedia.org/wiki/Window_function#Flat_top_window
 *
 *  Low resolution, high dynamic range.
 *
 */

#ifndef __FLATTOP_H__
#define __FLATTOP_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// flattop
//
struct flattop
{
	dsp_t dsp;												// parent class
	
	uint32_t size;										// window size
	double offset;										// adjustment
	double *window;										// window
};
typedef struct flattop flattop_t;





/**
 * flattop, window size, adjustment, pool
 */
int flattop_init (flattop_t*, uint32_t, double, opool_t*);

/**
 *
 */
int flattop_destroy (flattop_t*);





/**
 *
 */
int flattop_feed (flattop_t*, uint32_t*, double*);

/**
 *
 */
int flattop_reset (flattop_t*);





/**
 *
 */
flattop_t* flattop_retain (flattop_t*);

/**
 *
 */
void flattop_release (flattop_t*);

#endif /* __WINDOW_FLATTOP_H__ */
