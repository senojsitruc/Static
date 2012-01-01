/*
 *  ooura8.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.01.10.
 *  Copyright 2010 Curtis Jones. All rights reserved.
 *
 *  -----------------------------------------------------------------------------------------------
 *
 *  http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html
 *
 */

#ifndef __FFT_OOURA8_H__
#define __FFT_OOURA8_H__

#include <stdint.h>
#include "../fft.h"
#include "../../../misc/mem/opool.h"

//
// ooura8
//
struct ooura8
{
	fft_t fft;												// parent class
	
	int32_t *scratch;									// scratch space
	double *sincos;										// sine, cosine table
	double *data;											// fft data
};
typedef struct ooura8 ooura8_t;





/**
 *
 */
int ooura8_init (ooura8_t*, uint32_t, fft_direction, fft_type, opool_t*);

/**
 *
 */
int ooura8_destroy (ooura8_t*);





/**
 *
 */
ooura8_t* ooura8_retain (ooura8_t*);

/**
 *
 */
void ooura8_release (ooura8_t*);

#endif /* __FFT_OOURA8_H__ */
