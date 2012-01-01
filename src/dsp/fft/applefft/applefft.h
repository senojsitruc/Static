/*
 *  applefft.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.22.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __FFT_APPLEFFT_H__
#define __FFT_APPLEFFT_H__

#include <stdint.h>
#include "../fft.h"
#include "../../../misc/mem/opool.h"
#include <Accelerate/Accelerate.h>

//
// applefft
//
struct applefft
{
	fft_t fft;												// parent class
	
	FFTSetupD setup;                  // fft setup
	DSPDoubleSplitComplex data;       // fft data
};
typedef struct applefft applefft_t;





/**
 *
 */
int applefft_init (applefft_t*, uint32_t, fft_direction, fft_type, opool_t*);

/**
 *
 */
int applefft_destroy (applefft_t*);





/**
 *
 */
applefft_t* applefft_retain (applefft_t*);

/**
 *
 */
void applefft_release (applefft_t*);

#endif /* __FFT_APPLEFFT_H__ */
