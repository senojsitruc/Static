/*
 *  sinc.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.09.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  Low pass, brick-wall filter.
 *
 *  http://en.wikipedia.org/wiki/Sinc_filter
 *  http://www.dspguide.com/ch21/2.htm
 *  http://www.dspguide.com/ch16.htm
 *  http://en.wikipedia.org/wiki/Window_function
 *
 */

#ifndef __DSP_SINC_H__
#define __DSP_SINC_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// sinc_side
//
typedef enum
{
	SINC_SIDE_LOWER = (1<<0),
	SINC_SIDE_UPPER = (1<<1)
} sinc_side;

//
// sinc
//
struct sinc
{
	dsp_t dsp;												// parent class
	sinc_side side;										// sinc side (lower, upper)
};
typedef struct sinc sinc_t;





/**
 *
 */
int sinc_init (sinc_t*, sinc_side, opool_t*);

/**
 *
 */
int sinc_destroy (sinc_t*);





/**
 *
 */
int sinc_feed (sinc_t*, uint32_t*, double*);

/**
 *
 */
int sinc_reset (sinc_t*);





/**
 *
 */
sinc_t* sinc_retain (sinc_t*);

/**
 *
 */
void sinc_release (sinc_t*);

#endif /* __DSP_SINC_H__ */
