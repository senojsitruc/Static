/*
 *  fft.h
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.27.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#ifndef __DSP_FFT_H__
#define __DSP_FFT_H__

#include "../dsp.h"
#include "../../misc/mem/opool.h"

//
// fft_direction
//
typedef enum
{
	FFT_DIR_FORWARD  = (1 << 0),             // time to frequency domain
	FFT_DIR_BACKWARD = (1 << 1)              // frequency to time domain
} fft_direction;

//
// fft_type
//
typedef enum
{
	FFT_REAL    = (1 << 0),              // real numbers
	FFT_COMPLEX = (1 << 1)               // complex numbers
} fft_type;

//
// fft
//
struct fft
{
	dsp_t dsp;												// parent class
	
	uint32_t size;										// number of I/Q pairs
	uint32_t runs;										// number of times we've run
	fft_direction direction;					// fft direction (forward, bacward)
	fft_type type;                    // ftt type (real, complex)
	
	double *data_in;									// data in
	double *data_out;									// data out
	
	int (*__feed)(struct fft*, uint32_t*, double*);
	int (*__reset)(struct fft*);
};
typedef struct fft fft_t;

//
// __fft_feed_func
//
typedef int (*__fft_feed_func)(fft_t*, uint32_t*, double*);

//
// __fft_reset_func
//
typedef int (*__fft_reset_func)(fft_t*);





/**
 * fft, size, direction, type, name, destroy function pointer, pool
 */
int fft_init (fft_t*, uint32_t, fft_direction, fft_type, char*, cobject_destroy_func, opool_t*);

/**
 *
 */
int fft_destroy (fft_t*);





/**
 *
 */
double* fft_data_in (fft_t*);

/**
 *
 */
double* fft_data_out (fft_t*);





/**
 *
 */
int fft_feed (fft_t*, uint32_t*, double*);

/**
 *
 */
int fft_reset (fft_t*);





/**
 *
 */
fft_t* fft_retain (fft_t*);

/**
 *
 */
void fft_release (fft_t*);

#endif /* __DSP_FFT_H__ */
