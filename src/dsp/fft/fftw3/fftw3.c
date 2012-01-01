/*
 *  fftw3.c
 *  Static
 *
 *  Created by Curtis Jones on 2009.12.27.
 *  Copyright 2009 Curtis Jones. All rights reserved.
 *
 */

#include "fftw3.h"

/*
//#import <fftw3.h>

//fftw_complex *mFFTIn;							// 
//fftw_complex *mFFTOut;						// 
//fftw_plan mFFTPlan;								// 

//double *mFFTIn;										// 
//double *mFFTOut;									// 

if (mFFTIn != NULL)
fftw_free(mFFTIn);

if (mFFTOut != NULL)
fftw_free(mFFTOut);

//	mFFTIn[x][0] = samples[i];
//	mFFTIn[x][1] = samples[i+1];

//fftw_execute(mFFTPlan);

if (NULL == (mFFTIn = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*2048)))
NSLog(@"failed to malloc(mFFTIn), %s\n", strerror(errno));

if (NULL == (mFFTOut = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*2048)))
NSLog(@"failed to malloc(mFFTOut), %s\n", strerror(errno));

if (NULL == (mFFTIn = (double*)malloc(sizeof(double) * 4096)))
NSLog(@"failed to malloc(mFTTIn), %s\n", strerror(errno));

mFFTOut = mFFTIn;

memset(mFFTIn, 0, sizeof(fftw_complex)*2048);
memset(mFFTOut, 0, sizeof(fftw_complex)*2048);

 //	fftw_destroy_plan(mFFTPlan);

mFFTPlan = fftw_plan_dft_1d(2048, mFFTIn, mFFTOut, FFTW_FORWARD, FFTW_MEASURE);
*/
