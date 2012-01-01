/*
 *  average.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.30.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __AVERAGE_H__
#define __AVERAGE_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// average
//
struct average
{
	dsp_t dsp;												// parent class
	
	uint32_t data_size;								// size of data array
	uint32_t avg_size;								// period of average-ness
	uint32_t avg_count;								// number of samples
	
	double *sum;											// sum over period
	double *avg;											// current average
};
typedef struct average average_t;





/**
 * average, data size, average size, pool
 */
int average_init (average_t*, uint32_t, uint32_t, opool_t*);

/**
 *
 */
int average_destroy (average_t*);





/**
 *
 */
int average_feed (average_t*, uint32_t*, double*);

/**
 *
 */
int average_reset (average_t*);





/**
 *
 */
average_t* average_retain (average_t*);

/**
 *
 */
void average_release (average_t*);

#endif /* __AVERAGE_H__ */
