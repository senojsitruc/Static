/*
 *  baseband.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.12.03.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  Performs time domain complex down conversion to baseband of the frequency (in Hz) given at
 *  initialization.
 *
 */

#ifndef __DSP_BASEBAND_H__
#define __DSP_BASEBAND_H__

#include "../../dsp.h"
#include "../../../misc/mem/cobject.h"
#include "../../../misc/mem/opool.h"
#include <stdint.h>

//
// baseband
//
struct baseband
{
	dsp_t dsp;												// parent class
	
	uint32_t frequency;               // frequency (in Hz)
};
typedef struct baseband baseband_t;





/**
 * baseband, frequency (Hz), pool
 */
int baseband_init (baseband_t*, uint32_t, opool_t*);

/**
 *
 */
int baseband_destroy (baseband_t*);





/**
 *
 */
int baseband_feed (baseband_t*, uint32_t*, double*);

/**
 *
 */
int baseband_reset (baseband_t*);





/**
 *
 */
baseband_t* baseband_retain (baseband_t*);

/**
 *
 */
void baseband_release (baseband_t*);

#endif /* __DSP_BASEBAND_H__ */
