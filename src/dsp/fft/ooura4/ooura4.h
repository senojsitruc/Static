/*
 *  ooura4.h
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

#ifndef __FFT_OOURA4_H__
#define __FFT_OOURA4_H__

#include <stdint.h>
#include "../fft.h"
#include "../../../misc/mem/opool.h"

//
// ooura4
//
struct ooura4
{
	fft_t fft;												// parent class
	
	int32_t *scratch;									// scratch space
	double *sincos;										// sine, cosine table
	double *data;											// fft data
};
typedef struct ooura4 ooura4_t;





/**
 *
 */
int ooura4_init (ooura4_t*, uint32_t, fft_direction, fft_type, opool_t*);

/**
 *
 */
int ooura4_destroy (ooura4_t*);





/**
 *
 */
ooura4_t* ooura4_retain (ooura4_t*);

/**
 *
 */
void ooura4_release (ooura4_t*);

#endif /* __FFT_OOURA4_H__ */
