/*
 *  trim.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.20.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __DSP_TRIM_H__
#define __DSP_TRIM_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"

//
// dsptrim
//
struct dsptrim
{
	dsp_t dsp;                           // parent class
	
	uint32_t width;                      // data width (in bytes)
	uint32_t offset;                     // data offset (in bytes)
};
typedef struct dsptrim dsptrim_t;





/**
 * dsptrim, data width, data offset, pool
 */
int dsptrim_init (dsptrim_t*, uint32_t, uint32_t, opool_t*);

/**
 *
 */
int dsptrim_destroy (dsptrim_t*);





/**
 *
 */
int dsptrim_feed (dsptrim_t*, uint32_t*, uint8_t*);

/**
 *
 */
int dsptrim_reset (dsptrim_t*);





/**
 *
 */
dsptrim_t* dsptrim_retain (dsptrim_t*);

/**
 *
 */
void dsptrim_release (dsptrim_t*);

#endif /* __DSP_TRIM_H__ */
