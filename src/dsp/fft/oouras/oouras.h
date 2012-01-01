/*
 *  oouras.h
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.30.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
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
// oouras
//
struct oouras
{
	fft_t fft;												// parent class
	
	int32_t *scratch;									// scratch space
	double *sincos;										// sine, cosine table
	double *data;											// fft data
};
typedef struct oouras oouras_t;





/**
 *
 */
int oouras_init (oouras_t*, uint32_t, fft_direction, fft_type, opool_t*);

/**
 *
 */
int oouras_destroy (oouras_t*);





/**
 *
 */
oouras_t* oouras_retain (oouras_t*);

/**
 *
 */
void oouras_release (oouras_t*);

#endif /* __FFT_OOURA4_H__ */
