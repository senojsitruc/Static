/*
 *  applefft.c
 *  Static
 *
 *  Created by Curtis Jones on 2010.11.22.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "applefft.h"
#include "../../../misc/logger.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Accelerate/Accelerate.h>





#pragma mark -
#pragma mark private methods

static inline int __applefft_feed (applefft_t*, uint32_t*, DSPDoubleComplex*);
static inline int __applefft_reset (applefft_t*);





#pragma mark -
#pragma mark structors

/**
 * "size" is the number of values to expect in the call to feed(); the number of I's plus the 
 * number of Q's.
 *
 */
int
applefft_init (applefft_t *applefft, uint32_t size, fft_direction direction, fft_type type, opool_t *pool)
{
	int error;
	
	if (applefft == NULL)
		LOG_ERROR_AND_RETURN(-1, "null applefft_t");
	
	if (size > 65536)
		LOG_ERROR_AND_RETURN(-2, "invalid size (%u)", size);
	
	if (0 != (error = fft_init((fft_t*)applefft, size, direction, type, "dsp-fft-applefft", (cobject_destroy_func)applefft_destroy, pool)))
		LOG_ERROR_AND_RETURN(-101, "failed to fft_init, %d", error);
	
	if (NULL == (applefft->data.realp = malloc(size * sizeof(double) / 2)))
		LOG_ERROR_AND_RETURN(-102, "failed to malloc(), %s", strerror(errno));
	
	if (NULL == (applefft->data.imagp = malloc(size * sizeof(double) / 2)))
		LOG_ERROR_AND_RETURN(-103, "failed to malloc(), %s", strerror(errno));
	
	if (0 == (applefft->setup = vDSP_create_fftsetupD((vDSP_Length)log2(size/2), kFFTRadix2)))
		LOG_ERROR_AND_RETURN(-104, "failed to vDSP_create_fftsetup()");
	
	memset(applefft->data.realp, 0, size * sizeof(double) / 2);
	memset(applefft->data.imagp, 0, size * sizeof(double) / 2);
	
	applefft->fft.__feed = (__fft_feed_func)__applefft_feed;
	applefft->fft.__reset = (__fft_reset_func)__applefft_reset;
	
	return 0;
}

int
applefft_destroy (applefft_t *applefft)
{
	int error;
	
	if (applefft == NULL)
		LOG_ERROR_AND_RETURN(-1, "null applefft_t");
	
	// deallocates the fft stuff
	vDSP_destroy_fftsetupD(applefft->setup);
	
	if (0 != (error = fft_destroy((fft_t*)applefft)))
		LOG_ERROR_AND_RETURN(-101, "failed to fft_destroy, %d", error);
	
	return 0;
}





#pragma mark -
#pragma mark run

/**
 *
 *
 */
static inline int
__applefft_feed (applefft_t *applefft, uint32_t *size, DSPDoubleComplex *data)
{
	size_t count, bytes;
	
	if (unlikely(applefft == NULL))
		LOG_ERROR_AND_RETURN(-1, "null applefft_t");
	
	if (unlikely(size == NULL))
		LOG_ERROR_AND_RETURN(-2, "null size");
	
	if (unlikely(data == NULL))
		LOG_ERROR_AND_RETURN(-3, "null data");
	
	bytes = *size;
	count = bytes / sizeof(double);
	
	if (unlikely(bytes > applefft->fft.size * sizeof(double)))
		LOG_ERROR_AND_RETURN(-4, "mismatched sizes (size=%lu, applefft->fft.size=%lu)", bytes, (applefft->fft.size*sizeof(double)));
	
	memset(applefft->data.realp, 0, applefft->fft.size * sizeof(double) / 2);
	memset(applefft->data.imagp, 0, applefft->fft.size * sizeof(double) / 2);
	
	vDSP_ctozD(data, 2, &applefft->data, 1, count/2);
	
	// complex
	if (FFT_COMPLEX == applefft->fft.type) {
		if (FFT_DIR_FORWARD == applefft->fft.direction)
			vDSP_fft_zipD(applefft->setup, &applefft->data, 1, (vDSP_Length)log2(applefft->fft.size/2), kFFTDirection_Forward);
		else if (FFT_DIR_BACKWARD == applefft->fft.direction)
			vDSP_fft_zipD(applefft->setup, &applefft->data, 1, (vDSP_Length)log2(applefft->fft.size/2), kFFTDirection_Inverse);
	}
	
	/*
	// real
	else if (FFT_REAL == applefft->fft.type) {
		if (FFT_DIR_FORWARD == applefft->fft.direction)
			__applefft_rdft((int)applefft->fft.size, -1, applefft->data, applefft->scratch, applefft->sincos);
		else if (FFT_DIR_BACKWARD == applefft->fft.direction)
			__applefft_rdft((int)applefft->fft.size, 1, applefft->data, applefft->scratch, applefft->sincos);
	}
	*/
	
	bytes = sizeof(double) * applefft->fft.size;
	
	{
		DSPDoubleSplitComplex tmpdata;
		
		tmpdata.realp = applefft->data.realp;
		tmpdata.imagp = applefft->data.imagp;
		
		vDSP_ztocD(&tmpdata, 1, data+(applefft->fft.size/4), 2, applefft->fft.size/4);
		
		tmpdata.realp = applefft->data.realp + (applefft->fft.size / 4);
		tmpdata.imagp = applefft->data.imagp + (applefft->fft.size / 4);
		
		vDSP_ztocD(&tmpdata, 1, data, 2, applefft->fft.size/4);
	}
	
	/*
	// interleave and copy the fft result back into our buffer
	vDSP_ztocD(&applefft->data, 1, data, 2, applefft->fft.size/2);
	*/
	
	/*
	// the first shall become last and the last shall become first
	memcpy(data, applefft->data+(applefft->fft.size/2), _size / 2);
	memcpy(data+(applefft->fft.size/2), applefft->data, _size / 2);
	*/
	
	*size = (uint32_t)bytes;
	
	return 0;
}

/**
 *
 *
 */
static inline int
__applefft_reset (applefft_t *applefft)
{
	if (unlikely(applefft == NULL))
		LOG_ERROR_AND_RETURN(-1, "null applefft_t");
	
	memset(applefft->data.realp, 0, applefft->fft.size * sizeof(double) / 2);
	memset(applefft->data.imagp, 0, applefft->fft.size * sizeof(double) / 2);
	
	return 0;
}





#pragma mark -
#pragma mark cobject stuff

/**
 *
 *
 */
inline applefft_t*
applefft_retain (applefft_t *applefft)
{
	if (unlikely(applefft == NULL))
		LOG_ERROR_AND_RETURN(NULL, "null applefft_t");
	
	return (applefft_t*)fft_retain((fft_t*)applefft);
}

/**
 *
 *
 */
inline void
applefft_release (applefft_t *applefft)
{
	if (unlikely(applefft == NULL))
		LOG_ERROR_AND_RETURN(, "null applefft_t");
	
	fft_release((fft_t*)applefft);
}
