/*
 *  double.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.28.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __DSP_SCALE_DOUBLE_H__
#define __DSP_SCALE_DOUBLE_H__

#include "scale.h"
#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"
#include <stdint.h>

//
// scaledouble
//
struct scaledouble
{
	dsp_t dsp;                           // parent class
	
	dspscale_mode mode;                  // linear, log, exponential
	
	double src_hi;                       // source value high
	double src_lo;                       // source value low
	double dst_hi;                       // destination value high
	double dst_lo;                       // destination value lo
	
	double src_range;                    // source high - low
	double dst_range;                    // destination high - low
};
typedef struct scaledouble scaledouble_t;





/**
 * scaledouble, mode, src hi, src lo, dst hi, dst lo, pool
 */
int scaledouble_init (scaledouble_t*, dspscale_mode, double, double, double, double, opool_t*);

/**
 *
 */
int scaledouble_destroy (scaledouble_t*);





/**
 *
 */
int scaledouble_feed (scaledouble_t*, uint32_t*, double*);

/**
 *
 */
int scaledouble_reset (scaledouble_t*);





/**
 *
 */
scaledouble_t* scaledouble_retain (scaledouble_t*);

/**
 *
 */
void scaledouble_release (scaledouble_t*);

#endif /* __DSP_SCALE_DOUBLE_H__ */
