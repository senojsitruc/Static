/*
 *  smush.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.26.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  Compresses the data stream into a specific number of points.
 *
 */

#ifndef __DSP_SMUSH_H__
#define __DSP_SMUSH_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// dspsmush
//
struct dspsmush
{
	dsp_t dsp;												// parent class
	
	uint32_t width;                   // data width (in values - ie, doubles)
};
typedef struct dspsmush dspsmush_t;





/**
 * dspsmush, width, pool
 */
int dspsmush_init (dspsmush_t*, uint32_t, opool_t*);

/**
 *
 */
int dspsmush_destroy (dspsmush_t*);





/**
 *
 */
int dspsmush_feed (dspsmush_t*, uint32_t*, double*);

/**
 *
 */
int dspsmush_reset (dspsmush_t*);





/**
 *
 */
dspsmush_t* dspsmush_retain (dspsmush_t*);

/**
 *
 */
void dspsmush_release (dspsmush_t*);

#endif /* __DSP_SMUSH_H__ */
