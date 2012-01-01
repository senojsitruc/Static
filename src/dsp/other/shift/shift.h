/*
 *  shift.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.29.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __DSP_SHIFT_H__
#define __DSP_SHIFT_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"
#include <stdint.h>

//
// dspshift_dir
//
typedef enum
{
	DSPSHIFT_DIR_LEFT  = (1 << 0),       // left (down)
	DSPSHIFT_DIR_RIGHT = (1 << 1)        // right (up)
} dspshift_dir;

//
// dspshift
//
struct dspshift
{
	dsp_t dsp;                           // parent class
	
	dspshift_dir dir;                    // shift direction (left, right)
	uint32_t size;                       // number of bytes to shift
};
typedef struct dspshift dspshift_t;





/**
 * dspshift, direction, size, pool
 */
int dspshift_init (dspshift_t*, dspshift_dir, uint32_t, opool_t*);

/**
 *
 */
int dspshift_destroy (dspshift_t*);





/**
 *
 */
int dspshift_feed (dspshift_t*, uint32_t*, uint8_t*);

/**
 *
 */
int dspshift_reset (dspshift_t*);





/**
 *
 */
dspshift_t* dspshift_retain (dspshift_t*);

/**
 *
 */
void dspshift_release (dspshift_t*);

#endif /* __DSP_SHIFT_H__ */
