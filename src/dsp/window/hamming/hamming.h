/*
 *  hamming.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.27.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  http://en.wikipedia.org/wiki/Window_function#Hamming_window
 *
 */

#ifndef __HAMMING_H__
#define __HAMMING_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// hamming
//
struct hamming
{
	dsp_t dsp;												// parent class
	
	uint32_t size;										// window size
	double offset;										// adjustment
	double *window;										// window
};
typedef struct hamming hamming_t;





/**
 * hamming, window size, adjustment, pool
 */
int hamming_init (hamming_t*, uint32_t, double, opool_t*);

/**
 *
 */
int hamming_destroy (hamming_t*);





/**
 *
 */
int hamming_feed (hamming_t*, uint32_t*, double*);

/**
 *
 */
int hamming_reset (hamming_t*);





/**
 *
 */
hamming_t* hamming_retain (hamming_t*);

/**
 *
 */
void hamming_release (hamming_t*);

#endif /* __WINDOW_HAMMING_H__ */
