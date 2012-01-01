/*
 *  filter.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.14.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __FILTER_H__
#define __FILTER_H__

#include "../dsp.h"
#include "../../misc/mem/cobject.h"
#include "../../misc/mem/opool.h"
#include <stdint.h>





//
// filter_method
//
typedef enum
{
	FILTER_METHOD_RECURSE = (1<<0),		// combine signals by recursion
	FILTER_METHOD_CONVOLVE = (1<<1),	// combine signals by convolution
} filter_method;

//
// filter
//
struct filter
{
	dsp_t dsp;												// parent class
	
	filter_method method;							// filter method (recursion, convolution)
	
	double *output;										// signal output
	uint32_t output_l;								// output length
};
typedef struct filter filter_t;





/**
 * filter, method, output signal size, class name, destroy func, pool
 */
int filter_init (filter_t*, filter_method, uint32_t, char*, cobject_destroy_func, opool_t*);

/**
 *
 */
int filter_destroy (filter_t*);





/**
 * Combine signals via recursion.
 *
 * filter, poles, input signal length, input signal, filter coefficients 'a' length, filter 
 * coefficients 'a', filter coefficients 'b' length, filter coefficients 'b'
 */
int filter_recurse (filter_t*, uint32_t, uint32_t, double*, uint32_t, double*, uint32_t, double*);

/**
 * Combine signals via convolution.
 *
 * filter, input signal length, input signal, impulse length, impulse
 */
int filter_convolve (filter_t*, uint32_t, double*, uint32_t, double*);

/**
 * The filter_t does not explicitly add itself to the call path for reset(), thus it is the
 * responsibility of the subclass to call filter_reset() when appropriate.
 */
int filter_reset (filter_t*);





/**
 *
 */
filter_t* filter_retain (filter_t*);

/**
 *
 */
void filter_release (filter_t*);

#endif /* __FILTER_H__ */
