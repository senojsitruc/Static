/*
 *  zero.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.28.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __DSP_ZERO_H__
#define __DSP_ZERO_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// dspzero_mode
//
typedef enum
{
	DSPZERO_MODE_INCLUSIVE = (1 << 0),   // inclusive
	DSPZERO_MODE_EXCLUSIVE = (1 << 1)    // exclusive
} dspzero_mode;

//
// dspzero
//
struct dspzero
{
	dsp_t dsp;                           // parent class
	
	dspzero_mode mode;                   // mode: inclusive or exclusive
	uint32_t offset;                     // data offset (in bytes)
	uint32_t width;                      // data width (in bytes)
};
typedef struct dspzero dspzero_t;





/**
 * dspzero, mode, offset, width, pool
 */
int dspzero_init (dspzero_t*, dspzero_mode, uint32_t, uint32_t, opool_t*);

/**
 *
 */
int dspzero_destroy (dspzero_t*);





/**
 *
 */
int dspzero_feed (dspzero_t*, uint32_t*, uint8_t*);

/**
 *
 */
int dspzero_reset (dspzero_t*);





/**
 *
 */
dspzero_t* dspzero_retain (dspzero_t*);

/**
 *
 */
void dspzero_release (dspzero_t*);

#endif /* __DSP_ZERO_H__ */
